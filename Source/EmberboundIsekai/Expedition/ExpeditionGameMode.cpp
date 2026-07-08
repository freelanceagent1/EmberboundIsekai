#include "Expedition/ExpeditionGameMode.h"
#include "Combat/EmberPlayerCharacter.h"
#include "Combat/HealthComponent.h"
#include "Core/EconomySubsystem.h"
#include "Core/EmberGameInstance.h"
#include "Core/MonsterCollectionSubsystem.h"
#include "Core/SaveManagerSubsystem.h"
#include "Core/SceneFlowSubsystem.h"
#include "Data/EmberEnemyData.h"
#include "Enemies/BossTutorialDragon.h"
#include "Enemies/EmberEnemyBase.h"
#include "Enemies/EnemySpawnerSubsystem.h"
#include "Expedition/ExpeditionPortal.h"
#include "Run/EmberCompanionActor.h"
#include "Run/RunUpgradeManager.h"
#include "UI/EmberHUDWidget.h"
#include "UI/RunResultWidget.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"
#include "EmberboundIsekai.h"

namespace
{
	void SpawnExpeditionFloor(UWorld* World)
	{
		if (!World)
		{
			return;
		}
		UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
		UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Emberbound/Materials/Generated/MI_ExpeditionFloor_CorruptedGrass.MI_ExpeditionFloor_CorruptedGrass"));
		if (!PlaneMesh || !Material)
		{
			return;
		}
		AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(FVector(0.0f, 0.0f, -2.0f), FRotator::ZeroRotator);
		if (Floor && Floor->GetStaticMeshComponent())
		{
#if WITH_EDITOR
			Floor->SetActorLabel(TEXT("M5b_ExpeditionCorruptedGrassFloor"));
#endif
			Floor->GetStaticMeshComponent()->SetStaticMesh(PlaneMesh);
			Floor->GetStaticMeshComponent()->SetMaterial(0, Material);
			Floor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Floor->SetActorScale3D(FVector(28.0f, 28.0f, 1.0f));
		}
	}
}

static TAutoConsoleVariable<int32> CVarEmberStartRoom(
	TEXT("Ember.StartRoom"),
	1,
	TEXT("Start expedition at this 1-based room number."));

static TAutoConsoleVariable<float> CVarEmberPlayerDamageMult(
	TEXT("Ember.PlayerDamageMult"),
	1.0f,
	TEXT("Multiplier applied to player auto-attack projectile damage."));

static TAutoConsoleVariable<int32> CVarEmberDebugAutoDamageBoss(
	TEXT("Ember.DebugAutoDamageBoss"),
	0,
	TEXT("Development smoke hook: periodically apply Ember.DamageBoss 40 while a boss is alive."));

static TAutoConsoleVariable<float> CVarEmberDebugKillPlayerAfter(
	TEXT("Ember.DebugKillPlayerAfter"),
	0.0f,
	TEXT("Development smoke hook: kill the player after N seconds if N > 0."));

static TAutoConsoleVariable<int32> CVarEmberDebugAutoReturnCamp(
	TEXT("Ember.DebugAutoReturnCamp"),
	0,
	TEXT("Development smoke hook: return to camp after the run result appears."));

AExpeditionGameMode::AExpeditionGameMode()
{
	DefaultPawnClass = AEmberPlayerCharacter::StaticClass();
}

void AExpeditionGameMode::BeginPlay()
{
	Super::BeginPlay();
	SpawnExpeditionFloor(GetWorld());
	BuildRoomSequence();
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		HUDWidget = CreateWidget<UEmberHUDWidget>(PC, UEmberHUDWidget::StaticClass());
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}
	RegisterDebugHooks();
	BindPlayerDeath();
	SpawnAssignedCompanion();
	if (USaveManagerSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<USaveManagerSubsystem>() : nullptr)
	{
		UE_LOG(LogEmberbound, Log, TEXT("EMBER: PlayerDamage %.2f"), 12.0f * Save->GetPlayerDamageMultiplier());
	}
	FTimerHandle DelayedStartHandle;
	GetWorldTimerManager().SetTimer(DelayedStartHandle, this, &AExpeditionGameMode::StartCurrentRoom, 0.25f, false);
}

void AExpeditionGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterDebugHooks();
	Super::EndPlay(EndPlayReason);
}

void AExpeditionGameMode::BuildRoomSequence()
{
	Rooms.Reset();
	Rooms.Add({ { TEXT("GreenSlime"), TEXT("BoneRat") }, 2, 0.75f });
	Rooms.Add({ { TEXT("GreenSlime"), TEXT("RedSlime") }, 3, 0.75f });
	Rooms.Add({ { TEXT("GoblinStabber"), TEXT("GreenSlime") }, 4, 0.7f });
	Rooms.Add({ { TEXT("GoblinArcher"), TEXT("RedSlime"), TEXT("BoneRat") }, 5, 0.65f });
	Rooms.Add({ { TEXT("GoblinStabber"), TEXT("GoblinArcher"), TEXT("BoneRat") }, 6, 0.6f });
	Rooms.Add({ { TEXT("GoblinStabber") }, 2, 0.35f, EEmberRoomType::Elite });
	Rooms.Add({ {}, 1, 0.0f, EEmberRoomType::Boss });
}

void AExpeditionGameMode::StartCurrentRoom()
{
	if (bRunEnded)
	{
		return;
	}
	if (!bResolvedStartRoom)
	{
		CurrentRoomIndex = FMath::Clamp(CVarEmberStartRoom.GetValueOnGameThread(), 1, Rooms.Num()) - 1;
		bResolvedStartRoom = true;
		const float KillAfterSeconds = CVarEmberDebugKillPlayerAfter.GetValueOnGameThread();
		if (KillAfterSeconds > 0.0f)
		{
			bDebugKillPending = true;
			FTimerHandle KillPlayerTimerHandle;
			GetWorldTimerManager().SetTimer(KillPlayerTimerHandle, this, &AExpeditionGameMode::KillPlayerNow, KillAfterSeconds, false);
			GetWorldTimerManager().SetTimer(DebugKeepAliveTimerHandle, this, &AExpeditionGameMode::DebugKeepPlayerAlive, 0.2f, true);
		}
	}
	if (!Rooms.IsValidIndex(CurrentRoomIndex))
	{
		EndRun(true);
		return;
	}
	SpawnedThisRoom = 0;
	KilledThisRoom = 0;
	if (HUDWidget)
	{
		HUDWidget->SetRoom(CurrentRoomIndex + 1, Rooms.Num());
		HUDWidget->ClearBossHealthSource();
	}
	if (Rooms[CurrentRoomIndex].RoomType == EEmberRoomType::Boss)
	{
		SpawnBoss();
		if (CVarEmberDebugAutoDamageBoss.GetValueOnGameThread() != 0)
		{
			GetWorldTimerManager().SetTimer(AutoDamageBossTimerHandle, this, &AExpeditionGameMode::AutoDamageBossTick, 1.0f, true, 2.0f);
		}
		return;
	}
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AExpeditionGameMode::SpawnWaveEnemy, Rooms[CurrentRoomIndex].SpawnInterval, true, 0.2f);
	GetWorldTimerManager().SetTimer(RoomCheckTimerHandle, this, &AExpeditionGameMode::CheckRoomClear, 0.5f, true, 1.0f);
}

void AExpeditionGameMode::SpawnWaveEnemy()
{
	if (!Rooms.IsValidIndex(CurrentRoomIndex))
	{
		return;
	}
	const FEmberRoomWave& Room = Rooms[CurrentRoomIndex];
	UEnemySpawnerSubsystem* Registry = GetWorld()->GetSubsystem<UEnemySpawnerSubsystem>();
	if (SpawnedThisRoom >= Room.TotalToSpawn)
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		return;
	}
	if (Registry && Registry->GetLiveEnemyCount() >= MaxActiveEnemies)
	{
		return;
	}
	const FName EnemyName = Room.EnemyAssetNames[SpawnedThisRoom % Room.EnemyAssetNames.Num()];
	UEmberEnemyData* Data = LoadEnemyData(EnemyName);
	TSubclassOf<AEmberEnemyBase> EnemyClass = AEmberEnemyBase::StaticClass();
	if (Data && Data->EnemyClassOverride)
	{
		EnemyClass = Data->EnemyClassOverride;
	}
	AEmberEnemyBase* Enemy = GetWorld()->SpawnActor<AEmberEnemyBase>(EnemyClass, PickArenaEdgeSpawnLocation(), FRotator::ZeroRotator);
	if (Enemy)
	{
		Enemy->InitFromData(Data);
		if (Room.RoomType == EEmberRoomType::Elite)
		{
			Enemy->ApplyRuntimeModifiers(3.0f, 1.5f, 1.3f, FLinearColor(0.55f, 0.08f, 1.0f));
		}
		Enemy->OnAcquired_Implementation();
		++SpawnedThisRoom;
	}
}

void AExpeditionGameMode::SpawnBoss()
{
	ActiveBoss = GetWorld()->SpawnActor<ABossTutorialDragon>(ABossTutorialDragon::StaticClass(), FVector(520.0f, 0.0f, 90.0f), FRotator::ZeroRotator);
	if (ActiveBoss)
	{
		ActiveBoss->OnAcquired_Implementation();
		ActiveBoss->OnBossDefeated.AddUObject(this, &AExpeditionGameMode::HandleBossDefeated);
		if (HUDWidget)
		{
			HUDWidget->SetBossHealthSource(ActiveBoss->GetHealthComponent());
		}
	}
}

void AExpeditionGameMode::CheckRoomClear()
{
	if (!Rooms.IsValidIndex(CurrentRoomIndex))
	{
		GetWorldTimerManager().ClearTimer(RoomCheckTimerHandle);
		return;
	}
	UEnemySpawnerSubsystem* Registry = GetWorld()->GetSubsystem<UEnemySpawnerSubsystem>();
	if (SpawnedThisRoom >= Rooms[CurrentRoomIndex].TotalToSpawn && (!Registry || Registry->GetLiveEnemyCount() == 0))
	{
		CompleteRoom();
	}
}

void AExpeditionGameMode::CompleteRoom()
{
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	GetWorldTimerManager().ClearTimer(RoomCheckTimerHandle);
	GrantRoomReward();
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: RoomCleared Room=%d"), CurrentRoomIndex + 1);
	if (CurrentRoomIndex + 1 >= Rooms.Num())
	{
		EndRun(true);
		return;
	}
	SpawnPortal();
	FTimerHandle AutoAdvanceHandle;
	GetWorldTimerManager().SetTimer(AutoAdvanceHandle, this, &AExpeditionGameMode::EnterNextRoom, 1.0f, false);
}

void AExpeditionGameMode::SpawnPortal()
{
	if (CurrentPortal)
	{
		CurrentPortal->Destroy();
	}
	CurrentPortal = GetWorld()->SpawnActor<AExpeditionPortal>(AExpeditionPortal::StaticClass(), FVector(0.0f, 250.0f, 60.0f), FRotator::ZeroRotator);
	if (CurrentPortal)
	{
		CurrentPortal->OnPortalEntered.BindUObject(this, &AExpeditionGameMode::EnterNextRoom);
	}
}

void AExpeditionGameMode::EnterNextRoom()
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, 0.25f, FLinearColor::Black, false, true);
			PC->PlayerCameraManager->StartCameraFade(1.0f, 0.0f, 0.25f, FLinearColor::Black, false, false);
		}
	}
	if (CurrentPortal)
	{
		CurrentPortal->Destroy();
		CurrentPortal = nullptr;
	}
	++CurrentRoomIndex;
	StartCurrentRoom();
}

void AExpeditionGameMode::GrantRoomReward()
{
	float GoldMultiplier = 1.0f;
	if (URunUpgradeManager* UpgradeManager = GetWorld()->GetSubsystem<URunUpgradeManager>())
	{
		GoldMultiplier = UpgradeManager->GetGoldMultiplier();
	}
	const int32 RewardMultiplier = Rooms.IsValidIndex(CurrentRoomIndex) && Rooms[CurrentRoomIndex].RoomType == EEmberRoomType::Elite ? 2 : 1;
	AddRunResource(EEmberResource::Gold, FMath::Max(1, FMath::RoundToInt(static_cast<float>(FMath::RandRange(4, 8) * RewardMultiplier) * GoldMultiplier)));
	AddRunResource(EEmberResource::Wood, FMath::RandRange(1, 2) * RewardMultiplier);
	AddRunResource(EEmberResource::SoulAsh, RewardMultiplier);
}

void AExpeditionGameMode::AddRunResource(EEmberResource Resource, int32 Amount)
{
	if (Amount <= 0 || bRunEnded)
	{
		return;
	}
	RunResourceTally.FindOrAdd(Resource) += Amount;
}

void AExpeditionGameMode::AddRunMonsterShard(FName MonsterId, int32 Amount)
{
	if (Amount <= 0 || bRunEnded)
	{
		return;
	}
	RunMonsterShardTally.FindOrAdd(MonsterId) += Amount;
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: ShardDrop %s amount=%d total=%d"), *MonsterId.ToString(), Amount, RunMonsterShardTally.FindRef(MonsterId));
}

void AExpeditionGameMode::EndRun(bool bVictory)
{
	if (bRunEnded)
	{
		return;
	}
	bRunEnded = true;
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	GetWorldTimerManager().ClearTimer(RoomCheckTimerHandle);
	GetWorldTimerManager().ClearTimer(AutoDamageBossTimerHandle);
	GetWorldTimerManager().ClearTimer(DebugKeepAliveTimerHandle);
	if (!bVictory && ActiveBoss)
	{
		ActiveBoss->Destroy();
		ActiveBoss = nullptr;
	}
	if (AssignedCompanion)
	{
		AssignedCompanion->Destroy();
		AssignedCompanion = nullptr;
	}
	BankRunResult(bVictory);
	if (URunUpgradeManager* UpgradeManager = GetWorld()->GetSubsystem<URunUpgradeManager>())
	{
		UpgradeManager->ResetRun();
	}
	ShowRunResult();
}

void AExpeditionGameMode::BankRunResult(bool bVictory)
{
	UEmberGameInstance* EmberGI = Cast<UEmberGameInstance>(GetGameInstance());
	if (!EmberGI)
	{
		return;
	}
	FEmberRunResult Result;
	Result.bRunComplete = true;
	Result.bVictory = bVictory;
	Result.RoomsCleared = bVictory ? Rooms.Num() : CurrentRoomIndex;
	Result.EarnedResources = RunResourceTally;
	const float KeepFraction = bVictory ? 1.0f : 0.6f;
	for (const TPair<EEmberResource, int32>& Pair : RunResourceTally)
	{
		const int32 Banked = FMath::FloorToInt(static_cast<float>(Pair.Value) * KeepFraction);
		if (Banked > 0)
		{
			Result.BankedResources.Add(Pair.Key, Banked);
		}
	}
	if (bVictory)
	{
		Result.BankedResources.FindOrAdd(EEmberResource::Gold) += 25;
		Result.BankedResources.FindOrAdd(EEmberResource::Wood) += 10;
		Result.BankedResources.FindOrAdd(EEmberResource::SoulAsh) += 8;
		if (FMath::FRand() <= 0.3f)
		{
			Result.BankedResources.FindOrAdd(EEmberResource::Embers) += 1;
		}
		static const TArray<FName> BossShardIds = { TEXT("slime"), TEXT("bone-rat"), TEXT("skeleton-archer") };
		Result.MonsterShards.FindOrAdd(BossShardIds[FMath::RandRange(0, BossShardIds.Num() - 1)]) += FMath::RandRange(2, 3);
	}
	for (const TPair<FName, int32>& Pair : RunMonsterShardTally)
	{
		Result.MonsterShards.FindOrAdd(Pair.Key) += Pair.Value;
	}
	EmberGI->LastRunResult = Result;
	int32 TotalShards = 0;
	for (const TPair<FName, int32>& Pair : Result.MonsterShards)
	{
		TotalShards += Pair.Value;
	}
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: RunResult %s KeepPct=%d Gold=%d Wood=%d SoulAsh=%d Embers=%d Shards=%d"),
		bVictory ? TEXT("Victory") : TEXT("Defeat"),
		bVictory ? 100 : 60,
		Result.BankedResources.FindRef(EEmberResource::Gold),
		Result.BankedResources.FindRef(EEmberResource::Wood),
		Result.BankedResources.FindRef(EEmberResource::SoulAsh),
		Result.BankedResources.FindRef(EEmberResource::Embers),
		TotalShards);
}

void AExpeditionGameMode::ShowRunResult()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	UEmberGameInstance* EmberGI = Cast<UEmberGameInstance>(GetGameInstance());
	if (!PC || !EmberGI)
	{
		return;
	}
	ResultWidget = CreateWidget<URunResultWidget>(PC, URunResultWidget::StaticClass());
	if (ResultWidget)
	{
		ResultWidget->SetResult(EmberGI->LastRunResult);
		ResultWidget->AddToViewport(200);
		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeGameAndUI());
	}
	if (CVarEmberDebugAutoReturnCamp.GetValueOnGameThread() != 0)
	{
		FTimerHandle ReturnCampHandle;
		GetWorldTimerManager().SetTimer(ReturnCampHandle, [this]()
		{
			if (USceneFlowSubsystem* Flow = GetGameInstance() ? GetGameInstance()->GetSubsystem<USceneFlowSubsystem>() : nullptr)
			{
				Flow->TravelToCamp();
			}
		}, 1.0f, false);
	}
}

void AExpeditionGameMode::RegisterDebugHooks()
{
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Ember.KillPlayer"),
		TEXT("Kill the current expedition player."),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &AExpeditionGameMode::KillPlayerConsole),
		ECVF_Default);
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Ember.DamageBoss"),
		TEXT("Damage the active boss. Usage: Ember.DamageBoss 40"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &AExpeditionGameMode::DamageBossConsole),
		ECVF_Default);
}

void AExpeditionGameMode::UnregisterDebugHooks()
{
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("Ember.KillPlayer"), false);
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("Ember.DamageBoss"), false);
}

void AExpeditionGameMode::KillPlayerConsole(const TArray<FString>&)
{
	KillPlayerNow();
}

void AExpeditionGameMode::KillPlayerNow()
{
	bDebugKillPending = false;
	GetWorldTimerManager().ClearTimer(DebugKeepAliveTimerHandle);
	if (AEmberPlayerCharacter* Player = Cast<AEmberPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		if (UHealthComponent* Health = Player->GetHealthComponent())
		{
			Health->ApplyDamage(100000.0f);
		}
	}
}

void AExpeditionGameMode::DebugKeepPlayerAlive()
{
	if (!bDebugKillPending)
	{
		return;
	}
	if (AEmberPlayerCharacter* Player = Cast<AEmberPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		if (UHealthComponent* Health = Player->GetHealthComponent())
		{
			Health->Heal(1000.0f);
		}
	}
}

void AExpeditionGameMode::DamageBossConsole(const TArray<FString>& Args)
{
	const float Amount = Args.Num() > 0 ? FCString::Atof(*Args[0]) : 40.0f;
	if (ActiveBoss)
	{
		ActiveBoss->DamageBoss(Amount);
	}
}

void AExpeditionGameMode::AutoDamageBossTick()
{
	if (ActiveBoss && ActiveBoss->IsAlive())
	{
		ActiveBoss->DamageBoss(40.0f);
	}
	if (AEmberPlayerCharacter* Player = Cast<AEmberPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		if (UHealthComponent* Health = Player->GetHealthComponent())
		{
			Health->Heal(1000.0f);
		}
	}
}

void AExpeditionGameMode::BindPlayerDeath()
{
	if (AEmberPlayerCharacter* Player = Cast<AEmberPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		if (UHealthComponent* Health = Player->GetHealthComponent())
		{
			Health->OnDied.AddDynamic(this, &AExpeditionGameMode::HandlePlayerDied);
		}
	}
}

void AExpeditionGameMode::SpawnAssignedCompanion()
{
	UMonsterCollectionSubsystem* Monsters = GetGameInstance() ? GetGameInstance()->GetSubsystem<UMonsterCollectionSubsystem>() : nullptr;
	AEmberPlayerCharacter* Player = Cast<AEmberPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Monsters || !Player || !GetWorld())
	{
		return;
	}
	const FName MonsterId = Monsters->GetAssignedMonster(EMonsterAssignmentRole::Companion);
	UMonsterData* MonsterData = Monsters->FindMonster(MonsterId);
	if (!MonsterData)
	{
		return;
	}
	AssignedCompanion = GetWorld()->SpawnActor<AEmberCompanionActor>(AEmberCompanionActor::StaticClass(), Player->GetActorLocation() + FVector(120.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	if (AssignedCompanion)
	{
		AssignedCompanion->InitCompanion(MonsterData, Player);
	}
}

void AExpeditionGameMode::HandlePlayerDied(AActor*)
{
	EndRun(false);
}

void AExpeditionGameMode::HandleBossDefeated(ABossTutorialDragon*)
{
	if (HUDWidget)
	{
		HUDWidget->ClearBossHealthSource();
	}
	EndRun(true);
}

UEmberEnemyData* AExpeditionGameMode::LoadEnemyData(FName AssetName) const
{
	const FString Path = FString::Printf(TEXT("/Game/Emberbound/Data/Enemies/%s.%s"), *AssetName.ToString(), *AssetName.ToString());
	return LoadObject<UEmberEnemyData>(nullptr, *Path);
}

FVector AExpeditionGameMode::PickArenaEdgeSpawnLocation() const
{
	const float Extent = 760.0f;
	switch (FMath::RandRange(0, 3))
	{
	case 0:
		return FVector(-Extent, FMath::FRandRange(-Extent, Extent), 80.0f);
	case 1:
		return FVector(Extent, FMath::FRandRange(-Extent, Extent), 80.0f);
	case 2:
		return FVector(FMath::FRandRange(-Extent, Extent), -Extent, 80.0f);
	default:
		return FVector(FMath::FRandRange(-Extent, Extent), Extent, 80.0f);
	}
}

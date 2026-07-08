#include "Camp/DefenseEventManager.h"
#include "Camp/CampBuildingActor.h"
#include "Camp/EmberDefenderActor.h"
#include "Combat/AutoAttackComponent.h"
#include "Combat/EmberPlayerCharacter.h"
#include "Combat/HealthComponent.h"
#include "Core/EconomySubsystem.h"
#include "Core/EmberSaveGame.h"
#include "Core/MonsterCollectionSubsystem.h"
#include "Core/ObjectPoolSubsystem.h"
#include "Core/SaveManagerSubsystem.h"
#include "Data/EmberEnemyData.h"
#include "Data/MonsterData.h"
#include "Enemies/EmberEnemyBase.h"
#include "Enemies/EnemySpawnerSubsystem.h"
#include "UI/DefenseResultWidget.h"
#include "UI/DefenseStatusWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/App.h"
#include "Misc/CoreDelegates.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "EmberboundIsekai.h"

namespace
{
	UGameInstance* DefenseGameInstance(const UWorld* World)
	{
		return World ? World->GetGameInstance() : nullptr;
	}
}

void UDefenseEventManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RegisterDebugHooks();
}

void UDefenseEventManager::Deinitialize()
{
	UnregisterDebugHooks();
	CleanupEventActors();
	Super::Deinitialize();
}

bool UDefenseEventManager::StartDefense()
{
	if (bActive || !GetWorld())
	{
		return false;
	}
	CampfireActor = FindCampfireActor();
	if (!CampfireActor)
	{
		return false;
	}
	CampfireHealth = CampfireActor->FindComponentByClass<UHealthComponent>();
	if (!CampfireHealth)
	{
		CampfireHealth = NewObject<UHealthComponent>(CampfireActor, TEXT("DefenseCampfireHealth"));
		CampfireHealth->RegisterComponent();
		CampfireActor->AddInstanceComponent(CampfireHealth);
	}
	UGameInstance* GameInstance = DefenseGameInstance(GetWorld());
	const USaveManagerSubsystem* Save = GameInstance ? GameInstance->GetSubsystem<USaveManagerSubsystem>() : nullptr;
	const int32 CampfireLevel = Save ? Save->GetBuildingLevel(TEXT("campfire-core")) : 0;
	CampfireHealth->SetMaxHealth(200.0f + 50.0f * static_cast<float>(CampfireLevel), true);
	CampfireHealth->OnDamaged.AddDynamic(this, &UDefenseEventManager::HandleCampfireDamaged);
	CampfireHealth->OnDied.AddDynamic(this, &UDefenseEventManager::HandleCampfireDied);

	bActive = true;
	CurrentWave = 0;
	SpawnedThisWave = 0;
	PendingWaveEnemyIds.Reset();
	SpawnedEnemies.Reset();
	EnableCampAutoAttack(true);
	SpawnDefenders();
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		StatusWidget = CreateWidget<UDefenseStatusWidget>(PC, UDefenseStatusWidget::StaticClass());
		if (StatusWidget)
		{
			StatusWidget->AddToViewport(140);
		}
	}
	UpdateStatusWidget();
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: DefenseStarted"));
	BeginNextWave();
	return true;
}

void UDefenseEventManager::DamageCampfire(float Damage)
{
	if (CampfireHealth)
	{
		CampfireHealth->ApplyDamage(Damage);
	}
}

FDefenseRewardAmounts UDefenseEventManager::CalculateRewards(bool bVictory, int32 WatchtowerLevel)
{
	const float Bonus = 1.0f + 0.10f * static_cast<float>(FMath::Max(0, WatchtowerLevel));
	const float ResultMultiplier = bVictory ? 1.0f : 0.40f;
	FDefenseRewardAmounts Rewards;
	Rewards.Amounts.Add(EEmberResource::Gold, FMath::FloorToInt(15.0f * Bonus * ResultMultiplier));
	Rewards.Amounts.Add(EEmberResource::Wood, FMath::FloorToInt(6.0f * Bonus * ResultMultiplier));
	Rewards.Amounts.Add(EEmberResource::SoulAsh, FMath::FloorToInt(4.0f * Bonus * ResultMultiplier));
	Rewards.Amounts.Add(EEmberResource::MonsterEssence, FMath::FloorToInt(3.0f * Bonus * ResultMultiplier));
	return Rewards;
}

void UDefenseEventManager::RegisterDebugHooks()
{
	IConsoleManager& Console = IConsoleManager::Get();
	Console.RegisterConsoleCommand(TEXT("Ember.StartDefense"), TEXT("Start the camp defense event."), FConsoleCommandWithArgsDelegate::CreateUObject(this, &UDefenseEventManager::StartDefenseConsole), ECVF_Default);
	Console.RegisterConsoleCommand(TEXT("Ember.DebugAutoDefense"), TEXT("Auto-run camp defense. Use 1 for victory, 2 for wave-2 defeat."), FConsoleCommandWithArgsDelegate::CreateUObject(this, &UDefenseEventManager::DebugAutoDefenseConsole), ECVF_Default);
	Console.RegisterConsoleCommand(TEXT("Ember.DamageCampfire"), TEXT("Damage the campfire during defense."), FConsoleCommandWithArgsDelegate::CreateUObject(this, &UDefenseEventManager::DamageCampfireConsole), ECVF_Default);
}

void UDefenseEventManager::UnregisterDebugHooks()
{
	IConsoleManager& Console = IConsoleManager::Get();
	Console.UnregisterConsoleObject(TEXT("Ember.StartDefense"), false);
	Console.UnregisterConsoleObject(TEXT("Ember.DebugAutoDefense"), false);
	Console.UnregisterConsoleObject(TEXT("Ember.DamageCampfire"), false);
}

void UDefenseEventManager::StartDefenseConsole(const TArray<FString>&)
{
	StartDefense();
}

void UDefenseEventManager::DebugAutoDefenseConsole(const TArray<FString>& Args)
{
	const int32 Mode = Args.IsValidIndex(0) ? FCString::Atoi(*Args[0]) : 1;
	bDebugAutoDefense = Mode != 0;
	bDebugDefeatWave2 = Mode == 2;
	if (bDebugAutoDefense && !bActive)
	{
		StartDefense();
	}
}

void UDefenseEventManager::DamageCampfireConsole(const TArray<FString>& Args)
{
	DamageCampfire(Args.IsValidIndex(0) ? FCString::Atof(*Args[0]) : 0.0f);
}

void UDefenseEventManager::BeginNextWave()
{
	if (!bActive || !GetWorld())
	{
		return;
	}
	++CurrentWave;
	if (CurrentWave > 4)
	{
		FinishDefense(true);
		return;
	}
	BuildWave(CurrentWave, PendingWaveEnemyIds);
	SpawnedThisWave = 0;
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: DefenseWave n=%d"), CurrentWave);
	if (bDebugDefeatWave2 && CurrentWave == 2)
	{
		DamageCampfire(9999.0f);
		return;
	}
	SpawnNextEnemy();
	GetWorld()->GetTimerManager().SetTimer(PollTimerHandle, this, &UDefenseEventManager::PollWaveCompletion, 0.35f, true, 0.5f);
	if (bDebugAutoDefense)
	{
		GetWorld()->GetTimerManager().SetTimer(DebugTimerHandle, this, &UDefenseEventManager::DebugClearWaveEnemies, 0.75f, false);
	}
	UpdateStatusWidget();
}

void UDefenseEventManager::SpawnNextEnemy()
{
	if (!bActive || !GetWorld() || PendingWaveEnemyIds.Num() <= 0)
	{
		return;
	}
	UEnemySpawnerSubsystem* Spawner = GetWorld()->GetSubsystem<UEnemySpawnerSubsystem>();
	UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
	if (!Pool || (Spawner && Spawner->GetLiveEnemyCount() >= 20))
	{
		GetWorld()->GetTimerManager().SetTimer(WaveTimerHandle, this, &UDefenseEventManager::SpawnNextEnemy, 0.25f, false);
		return;
	}
	const FName EnemyId = PendingWaveEnemyIds[0];
	PendingWaveEnemyIds.RemoveAt(0);
	UEmberEnemyData* EnemyData = LoadEnemyData(EnemyId);
	if (!EnemyData)
	{
		SpawnNextEnemy();
		return;
	}
	static const FVector LaneStarts[] = {
		FVector(0.0f, 980.0f, 70.0f),
		FVector(980.0f, 0.0f, 70.0f),
		FVector(-980.0f, 0.0f, 70.0f)
	};
	const FVector SpawnLocation = LaneStarts[SpawnedThisWave % UE_ARRAY_COUNT(LaneStarts)];
	if (AEmberEnemyBase* Enemy = Pool->Acquire<AEmberEnemyBase>(AEmberEnemyBase::StaticClass(), FTransform(FRotator::ZeroRotator, SpawnLocation)))
	{
		Enemy->InitFromData(EnemyData);
		Enemy->SetForcedTarget(CampfireActor);
		if (CurrentWave == 4 && EnemyId == TEXT("GoblinStabber"))
		{
			Enemy->ApplyRuntimeModifiers(2.3f, 1.6f, 1.35f, FLinearColor(1.0f, 0.35f, 0.12f, 1.0f));
		}
		SpawnedEnemies.Add(Enemy);
		++SpawnedThisWave;
	}
	if (PendingWaveEnemyIds.Num() > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(WaveTimerHandle, this, &UDefenseEventManager::SpawnNextEnemy, 0.18f, false);
	}
}

void UDefenseEventManager::PollWaveCompletion()
{
	if (!bActive || !GetWorld())
	{
		return;
	}
	if (PendingWaveEnemyIds.Num() > 0)
	{
		return;
	}
	UEnemySpawnerSubsystem* Spawner = GetWorld()->GetSubsystem<UEnemySpawnerSubsystem>();
	if (Spawner && Spawner->GetLiveEnemyCount() > 0)
	{
		return;
	}
	GetWorld()->GetTimerManager().ClearTimer(PollTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(WaveTimerHandle, this, &UDefenseEventManager::BeginNextWave, 3.0f, false);
}

void UDefenseEventManager::FinishDefense(bool bVictory)
{
	if (!bActive)
	{
		return;
	}
	bActive = false;
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(WaveTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(PollTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(DebugTimerHandle);
	}
	UGameInstance* GameInstance = DefenseGameInstance(GetWorld());
	const USaveManagerSubsystem* Save = GameInstance ? GameInstance->GetSubsystem<USaveManagerSubsystem>() : nullptr;
	UEconomySubsystem* Economy = GameInstance ? GameInstance->GetSubsystem<UEconomySubsystem>() : nullptr;
	USaveManagerSubsystem* SaveMutable = GameInstance ? GameInstance->GetSubsystem<USaveManagerSubsystem>() : nullptr;
	const int32 WatchtowerLevel = Save ? Save->GetBuildingLevel(TEXT("watchtower")) : 0;
	const FDefenseRewardAmounts Rewards = CalculateRewards(bVictory, WatchtowerLevel);
	if (Economy)
	{
		for (const TPair<EEmberResource, int32>& Pair : Rewards.Amounts)
		{
			Economy->Add(Pair.Key, Pair.Value);
		}
	}
	if (SaveMutable)
	{
		SaveMutable->SaveNow();
	}
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		UDefenseResultWidget* ResultWidget = CreateWidget<UDefenseResultWidget>(PC, UDefenseResultWidget::StaticClass());
		if (ResultWidget)
		{
			ResultWidget->SetResult(bVictory, Rewards.Amounts);
			ResultWidget->AddToViewport(170);
		}
	}
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: DefenseResult %s gold=%d wood=%d soulash=%d essence=%d"),
		bVictory ? TEXT("Victory") : TEXT("Defeat"),
		Rewards.Amounts.FindRef(EEmberResource::Gold),
		Rewards.Amounts.FindRef(EEmberResource::Wood),
		Rewards.Amounts.FindRef(EEmberResource::SoulAsh),
		Rewards.Amounts.FindRef(EEmberResource::MonsterEssence));
	EnableCampAutoAttack(false);
	CleanupEventActors();
	if (bDebugAutoDefense && FApp::IsUnattended() && GetWorld())
	{
		FTimerHandle ExitHandle;
		GetWorld()->GetTimerManager().SetTimer(ExitHandle, []()
		{
			FGenericPlatformMisc::RequestExit(false);
		}, 0.75f, false);
	}
}

void UDefenseEventManager::SpawnDefenders()
{
	UGameInstance* GameInstance = DefenseGameInstance(GetWorld());
	USaveManagerSubsystem* Save = GameInstance ? GameInstance->GetSubsystem<USaveManagerSubsystem>() : nullptr;
	UMonsterCollectionSubsystem* Monsters = GameInstance ? GameInstance->GetSubsystem<UMonsterCollectionSubsystem>() : nullptr;
	UObjectPoolSubsystem* Pool = GetWorld() ? GetWorld()->GetSubsystem<UObjectPoolSubsystem>() : nullptr;
	if (!Save || !Save->GetCurrentSave() || !Monsters || !Pool)
	{
		return;
	}
	int32 DefenderIndex = 0;
	for (const FMonsterAssignmentSave& Assignment : Save->GetCurrentSave()->Assignments)
	{
		if (Assignment.AssignmentId != TEXT("defender"))
		{
			continue;
		}
		UMonsterData* MonsterData = Monsters->FindMonster(Assignment.MonsterId);
		if (!MonsterData)
		{
			continue;
		}
		const float Angle = 120.0f * static_cast<float>(DefenderIndex);
		const FVector Location = FVector(260.0f, 0.0f, 70.0f).RotateAngleAxis(Angle, FVector::UpVector);
		if (AEmberDefenderActor* Defender = Pool->Acquire<AEmberDefenderActor>(AEmberDefenderActor::StaticClass(), FTransform(FRotator::ZeroRotator, Location)))
		{
			Defender->InitDefender(MonsterData);
			Defenders.Add(Defender);
			UE_LOG(LogEmberbound, Log, TEXT("EMBER: DefenderSpawned %s"), *Assignment.MonsterId.ToString());
			++DefenderIndex;
		}
	}
}

void UDefenseEventManager::CleanupEventActors()
{
	if (StatusWidget)
	{
		StatusWidget->RemoveFromParent();
		StatusWidget = nullptr;
	}
	if (CampfireHealth)
	{
		CampfireHealth->OnDamaged.RemoveDynamic(this, &UDefenseEventManager::HandleCampfireDamaged);
		CampfireHealth->OnDied.RemoveDynamic(this, &UDefenseEventManager::HandleCampfireDied);
	}
	UObjectPoolSubsystem* Pool = GetWorld() ? GetWorld()->GetSubsystem<UObjectPoolSubsystem>() : nullptr;
	for (AEmberDefenderActor* Defender : Defenders)
	{
		if (Defender && Pool)
		{
			Pool->ReleaseActor(Defender);
		}
	}
	Defenders.Reset();
	for (AEmberEnemyBase* Enemy : SpawnedEnemies)
	{
		if (Enemy && Enemy->IsAlive() && Pool)
		{
			Pool->ReleaseActor(Enemy);
		}
	}
	SpawnedEnemies.Reset();
	CampfireHealth = nullptr;
	CampfireActor = nullptr;
	PendingWaveEnemyIds.Reset();
}

void UDefenseEventManager::EnableCampAutoAttack(bool bEnabled)
{
	if (AEmberPlayerCharacter* Player = Cast<AEmberPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
	{
		Player->SetAutoAttackEnabled(bEnabled);
	}
}

void UDefenseEventManager::UpdateStatusWidget()
{
	if (StatusWidget && CampfireHealth)
	{
		StatusWidget->SetStatus(CurrentWave, CampfireHealth->GetCurrentHP(), CampfireHealth->GetMaxHP());
	}
}

void UDefenseEventManager::DebugClearWaveEnemies()
{
	for (AEmberEnemyBase* Enemy : SpawnedEnemies)
	{
		if (Enemy && Enemy->IsAlive())
		{
			if (UHealthComponent* Health = Enemy->GetHealthComponent())
			{
				Health->ApplyDamage(99999.0f);
			}
		}
	}
}

AActor* UDefenseEventManager::FindCampfireActor() const
{
	for (TActorIterator<ACampfireCoreActor> It(GetWorld()); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

UEmberEnemyData* UDefenseEventManager::LoadEnemyData(FName EnemyId) const
{
	const TMap<FName, FString> AssetNames = {
		{ TEXT("GreenSlime"), TEXT("GreenSlime") },
		{ TEXT("RedSlime"), TEXT("RedSlime") },
		{ TEXT("BoneRat"), TEXT("BoneRat") },
		{ TEXT("GoblinStabber"), TEXT("GoblinStabber") },
		{ TEXT("GoblinArcher"), TEXT("GoblinArcher") }
	};
	const FString AssetName = AssetNames.Contains(EnemyId) ? AssetNames[EnemyId] : EnemyId.ToString();
	const FString Path = FString::Printf(TEXT("/Game/Emberbound/Data/Enemies/%s.%s"), *AssetName, *AssetName);
	return LoadObject<UEmberEnemyData>(nullptr, *Path);
}

void UDefenseEventManager::BuildWave(int32 WaveNumber, TArray<FName>& OutEnemyIds) const
{
	OutEnemyIds.Reset();
	if (WaveNumber == 1)
	{
		for (int32 Index = 0; Index < 6; ++Index)
		{
			OutEnemyIds.Add(Index % 2 == 0 ? TEXT("GreenSlime") : TEXT("RedSlime"));
		}
	}
	else if (WaveNumber == 2)
	{
		for (int32 Index = 0; Index < 8; ++Index)
		{
			OutEnemyIds.Add(TEXT("BoneRat"));
		}
	}
	else if (WaveNumber == 3)
	{
		for (int32 Index = 0; Index < 6; ++Index)
		{
			OutEnemyIds.Add(TEXT("GoblinStabber"));
		}
		OutEnemyIds.Add(TEXT("GoblinArcher"));
		OutEnemyIds.Add(TEXT("GoblinArcher"));
	}
	else if (WaveNumber == 4)
	{
		OutEnemyIds.Add(TEXT("GoblinStabber"));
		for (int32 Index = 0; Index < 4; ++Index)
		{
			OutEnemyIds.Add(TEXT("BoneRat"));
		}
	}
}

void UDefenseEventManager::HandleCampfireDamaged(float, float)
{
	UpdateStatusWidget();
}

void UDefenseEventManager::HandleCampfireDied(AActor*)
{
	FinishDefense(false);
}

#include "Expedition/ExpeditionGameMode.h"
#include "Combat/EmberPlayerCharacter.h"
#include "Core/EconomySubsystem.h"
#include "Data/EmberEnemyData.h"
#include "Enemies/EmberEnemyBase.h"
#include "Enemies/EnemySpawnerSubsystem.h"
#include "Expedition/ExpeditionPortal.h"
#include "Run/RunUpgradeManager.h"
#include "UI/EmberHUDWidget.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "EmberboundIsekai.h"

AExpeditionGameMode::AExpeditionGameMode()
{
	DefaultPawnClass = AEmberPlayerCharacter::StaticClass();
}

void AExpeditionGameMode::BeginPlay()
{
	Super::BeginPlay();
	BuildRoomSequence();
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		HUDWidget = CreateWidget<UEmberHUDWidget>(PC, UEmberHUDWidget::StaticClass());
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}
	StartCurrentRoom();
}

void AExpeditionGameMode::BuildRoomSequence()
{
	Rooms.Reset();
	Rooms.Add({ { TEXT("GreenSlime"), TEXT("BoneRat") }, 2, 0.75f });
	Rooms.Add({ { TEXT("GreenSlime"), TEXT("RedSlime") }, 3, 0.75f });
	Rooms.Add({ { TEXT("GoblinStabber"), TEXT("GreenSlime") }, 4, 0.7f });
	Rooms.Add({ { TEXT("GoblinArcher"), TEXT("RedSlime"), TEXT("BoneRat") }, 5, 0.65f });
	Rooms.Add({ { TEXT("GoblinStabber"), TEXT("GoblinArcher"), TEXT("BoneRat") }, 6, 0.6f });
}

void AExpeditionGameMode::StartCurrentRoom()
{
	if (!Rooms.IsValidIndex(CurrentRoomIndex))
	{
		UE_LOG(LogEmberbound, Log, TEXT("EMBER: RUN COMPLETE"));
		if (URunUpgradeManager* UpgradeManager = GetWorld()->GetSubsystem<URunUpgradeManager>())
		{
			UpgradeManager->ResetRun();
		}
		return;
	}
	SpawnedThisRoom = 0;
	KilledThisRoom = 0;
	if (HUDWidget)
	{
		HUDWidget->SetRoom(CurrentRoomIndex + 1, Rooms.Num());
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
		Enemy->OnAcquired_Implementation();
		++SpawnedThisRoom;
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
		UE_LOG(LogEmberbound, Log, TEXT("EMBER: RUN COMPLETE"));
		if (URunUpgradeManager* UpgradeManager = GetWorld()->GetSubsystem<URunUpgradeManager>())
		{
			UpgradeManager->ResetRun();
		}
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
	if (UEconomySubsystem* Economy = GetGameInstance()->GetSubsystem<UEconomySubsystem>())
	{
		float GoldMultiplier = 1.0f;
		if (URunUpgradeManager* UpgradeManager = GetWorld()->GetSubsystem<URunUpgradeManager>())
		{
			GoldMultiplier = UpgradeManager->GetGoldMultiplier();
		}
		Economy->Add(EEmberResource::Gold, FMath::Max(1, FMath::RoundToInt(static_cast<float>(FMath::RandRange(4, 8)) * GoldMultiplier)));
		Economy->Add(EEmberResource::Wood, FMath::RandRange(1, 2));
		Economy->Add(EEmberResource::SoulAsh, 1);
	}
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

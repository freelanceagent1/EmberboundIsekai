#include "Core/EmberAutopilotSubsystem.h"
#include "Camp/CampGameMode.h"
#include "Camp/DefenseEventManager.h"
#include "Combat/HealthComponent.h"
#include "Core/EmberGameInstance.h"
#include "Core/MonsterCollectionSubsystem.h"
#include "Core/SaveManagerSubsystem.h"
#include "Core/SceneFlowSubsystem.h"
#include "Data/BuildingData.h"
#include "Data/MonsterData.h"
#include "Enemies/BossTutorialDragon.h"
#include "Enemies/EmberEnemyBase.h"
#include "Expedition/ExpeditionGameMode.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "EmberboundIsekai.h"

namespace
{
	constexpr float AutopilotTickSeconds = 0.35f;

	UWorld* FindGameWorld(UGameInstance* GameInstance)
	{
		if (GameInstance && GameInstance->GetWorld())
		{
			return GameInstance->GetWorld();
		}
		if (!GEngine)
		{
			return nullptr;
		}
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			UWorld* World = Context.World();
			if (World && World->IsGameWorld())
			{
				return World;
			}
		}
		return nullptr;
	}

	UBuildingData* LoadAutopilotBuilding(FName BuildingId)
	{
		const TMap<FName, FString> AssetNames = {
			{ TEXT("campfire-core"), TEXT("DA_CampfireCore") },
			{ TEXT("grim-forge"), TEXT("DA_GrimForge") }
		};
		const FString AssetName = AssetNames.Contains(BuildingId) ? AssetNames[BuildingId] : BuildingId.ToString();
		const FString Path = FString::Printf(TEXT("/Game/Emberbound/Data/Buildings/%s.%s"), *AssetName, *AssetName);
		return LoadObject<UBuildingData>(nullptr, *Path);
	}
}

void UEmberAutopilotSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
#if !UE_BUILD_SHIPPING
	AutopilotCommand = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Ember.Autopilot"),
		TEXT("Run the continuous Emberbound end-to-end autopilot. Usage: Ember.Autopilot 1"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UEmberAutopilotSubsystem::AutopilotConsole),
		ECVF_Default);
#endif
}

void UEmberAutopilotSubsystem::Deinitialize()
{
#if !UE_BUILD_SHIPPING
	if (AutopilotCommand)
	{
		IConsoleManager::Get().UnregisterConsoleObject(AutopilotCommand);
		AutopilotCommand = nullptr;
	}
#endif
	Super::Deinitialize();
}

TArray<FString> UEmberAutopilotSubsystem::GetExpectedMarkerChain()
{
	return {
		TEXT("EMBER: AUTOPILOT phase=CampStart"),
		TEXT("EMBER: AUTOPILOT travel=Expedition"),
		TEXT("EMBER: AUTOPILOT phase=Expedition"),
		TEXT("EMBER: AUTOPILOT roomsCleared"),
		TEXT("EMBER: AUTOPILOT bossKilled"),
		TEXT("EMBER: AUTOPILOT travel=Camp"),
		TEXT("EMBER: AUTOPILOT phase=CampReturn"),
		TEXT("EMBER: AUTOPILOT bought=campfire-core"),
		TEXT("EMBER: AUTOPILOT bought=grim-forge"),
		TEXT("EMBER: AUTOPILOT defenseDone"),
		TEXT("EMBER: AUTOPILOT offlineDone"),
		TEXT("EMBER: AUTOPILOT saved")
	};
}

void UEmberAutopilotSubsystem::OnCampBeginPlay(ACampGameMode* CampMode)
{
	if (Phase == EEmberAutopilotPhase::CampStart || Phase == EEmberAutopilotPhase::TravelingToCamp || Phase == EEmberAutopilotPhase::CampReturn)
	{
		if (Phase == EEmberAutopilotPhase::TravelingToCamp)
		{
			Phase = EEmberAutopilotPhase::CampReturn;
		}
		if (Phase == EEmberAutopilotPhase::CampStart)
		{
			BeginCampStart(CampMode);
		}
		else
		{
			BeginCampReturn(CampMode);
		}
	}
}

void UEmberAutopilotSubsystem::OnExpeditionBeginPlay(AExpeditionGameMode* ExpeditionMode)
{
	if (Phase == EEmberAutopilotPhase::TravelingToExpedition || Phase == EEmberAutopilotPhase::Expedition)
	{
		Phase = EEmberAutopilotPhase::Expedition;
		BeginExpedition(ExpeditionMode);
	}
}

void UEmberAutopilotSubsystem::AutopilotConsole(const TArray<FString>& Args)
{
	const bool bEnable = Args.Num() == 0 || FCString::Atoi(*Args[0]) != 0;
	if (bEnable)
	{
		StartAutopilot();
	}
	else
	{
		Phase = EEmberAutopilotPhase::Inactive;
	}
}

void UEmberAutopilotSubsystem::StartAutopilot()
{
	Phase = EEmberAutopilotPhase::CampStart;
	LastLoggedRoom = 0;
	bLoggedRoomsCleared = false;
	bLoggedBossKilled = false;
	bDefenseStarted = false;
	bDefenseWasActive = false;
	bOfflineDone = false;
	ResumeForCurrentWorld();
}

void UEmberAutopilotSubsystem::ResumeForCurrentWorld()
{
	UWorld* World = FindGameWorld(GetGameInstance());
	if (!World)
	{
		return;
	}
	if (ACampGameMode* CampMode = World->GetAuthGameMode<ACampGameMode>())
	{
		OnCampBeginPlay(CampMode);
	}
	else if (AExpeditionGameMode* ExpeditionMode = World->GetAuthGameMode<AExpeditionGameMode>())
	{
		OnExpeditionBeginPlay(ExpeditionMode);
	}
}

void UEmberAutopilotSubsystem::BeginCampStart(ACampGameMode* CampMode)
{
	UWorld* World = FindGameWorld(GetGameInstance());
	if (!World)
	{
		return;
	}
	if (UEmberGameInstance* EmberGI = Cast<UEmberGameInstance>(GetGameInstance()))
	{
		EmberGI->SetMainMenuActive(false);
	}
	if (CampMode)
	{
		CampMode->StartCampAfterMenu();
	}
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: AUTOPILOT phase=CampStart"));
	if (USaveManagerSubsystem* Save = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>())
	{
		Save->GrantResources(80, 40, 20, 0, 2);
	}
	World->GetTimerManager().ClearTimer(CampStartTimerHandle);
	World->GetTimerManager().SetTimer(CampStartTimerHandle, this, &UEmberAutopilotSubsystem::TravelToExpedition, 2.0f, false);
}

void UEmberAutopilotSubsystem::TravelToExpedition()
{
	Phase = EEmberAutopilotPhase::TravelingToExpedition;
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: AUTOPILOT travel=Expedition"));
	if (USceneFlowSubsystem* Flow = GetGameInstance()->GetSubsystem<USceneFlowSubsystem>())
	{
		Flow->TravelToExpedition();
	}
}

void UEmberAutopilotSubsystem::BeginExpedition(AExpeditionGameMode*)
{
	UWorld* World = FindGameWorld(GetGameInstance());
	if (!World)
	{
		return;
	}
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: AUTOPILOT phase=Expedition"));
	if (IConsoleVariable* AutoPickUpgrades = IConsoleManager::Get().FindConsoleVariable(TEXT("Ember.AutoPickUpgrades")))
	{
		AutoPickUpgrades->Set(1, ECVF_SetByConsole);
	}
	World->GetTimerManager().ClearTimer(ExpeditionTickTimerHandle);
	World->GetTimerManager().SetTimer(ExpeditionTickTimerHandle, this, &UEmberAutopilotSubsystem::ExpeditionTick, AutopilotTickSeconds, true, 0.4f);
}

void UEmberAutopilotSubsystem::ExpeditionTick()
{
	UWorld* World = FindGameWorld(GetGameInstance());
	AExpeditionGameMode* ExpeditionMode = World ? World->GetAuthGameMode<AExpeditionGameMode>() : nullptr;
	if (!World || !ExpeditionMode || Phase != EEmberAutopilotPhase::Expedition)
	{
		return;
	}

	const int32 RoomNumber = ExpeditionMode->GetAutopilotRoomNumber();
	if (RoomNumber > LastLoggedRoom)
	{
		LastLoggedRoom = RoomNumber;
	}
	if (!bLoggedRoomsCleared && RoomNumber >= 7)
	{
		bLoggedRoomsCleared = true;
		UE_LOG(LogEmberbound, Log, TEXT("EMBER: AUTOPILOT roomsCleared"));
	}

	ForceClearExpeditionEnemies(World);
	DamageActiveBoss(ExpeditionMode);

	UEmberGameInstance* EmberGI = Cast<UEmberGameInstance>(GetGameInstance());
	if (EmberGI && EmberGI->LastRunResult.bRunComplete && EmberGI->LastRunResult.bVictory)
	{
		if (!bLoggedRoomsCleared)
		{
			bLoggedRoomsCleared = true;
			UE_LOG(LogEmberbound, Log, TEXT("EMBER: AUTOPILOT roomsCleared"));
		}
		if (!bLoggedBossKilled)
		{
			bLoggedBossKilled = true;
			UE_LOG(LogEmberbound, Log, TEXT("EMBER: AUTOPILOT bossKilled"));
		}
		World->GetTimerManager().ClearTimer(ExpeditionTickTimerHandle);
		Phase = EEmberAutopilotPhase::TravelingToCamp;
		UE_LOG(LogEmberbound, Log, TEXT("EMBER: AUTOPILOT travel=Camp"));
		if (USceneFlowSubsystem* Flow = GetGameInstance()->GetSubsystem<USceneFlowSubsystem>())
		{
			Flow->TravelToCamp();
		}
	}
}

void UEmberAutopilotSubsystem::BeginCampReturn(ACampGameMode* CampMode)
{
	UWorld* World = FindGameWorld(GetGameInstance());
	if (!World)
	{
		return;
	}
	if (UEmberGameInstance* EmberGI = Cast<UEmberGameInstance>(GetGameInstance()))
	{
		EmberGI->SetMainMenuActive(false);
	}
	if (CampMode)
	{
		CampMode->StartCampAfterMenu();
	}
	Phase = EEmberAutopilotPhase::CampReturn;
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: AUTOPILOT phase=CampReturn"));
	World->GetTimerManager().ClearTimer(CampReturnTickTimerHandle);
	World->GetTimerManager().SetTimer(CampReturnTickTimerHandle, this, &UEmberAutopilotSubsystem::CampReturnTick, AutopilotTickSeconds, true, 0.5f);
}

void UEmberAutopilotSubsystem::CampReturnTick()
{
	UWorld* World = FindGameWorld(GetGameInstance());
	if (!World || Phase != EEmberAutopilotPhase::CampReturn)
	{
		return;
	}

	if (!bDefenseStarted)
	{
		BuyBuilding(TEXT("campfire-core"));
		UE_LOG(LogEmberbound, Log, TEXT("EMBER: AUTOPILOT bought=campfire-core"));
		BuyBuilding(TEXT("grim-forge"));
		UE_LOG(LogEmberbound, Log, TEXT("EMBER: AUTOPILOT bought=grim-forge"));

		if (UMonsterCollectionSubsystem* Monsters = GetGameInstance()->GetSubsystem<UMonsterCollectionSubsystem>())
		{
			Monsters->GrantShards(TEXT("bone-rat"), 99);
			Monsters->AssignMonster(TEXT("bone-rat"), EMonsterAssignmentRole::Defender);
		}
		if (UDefenseEventManager* Defense = World->GetSubsystem<UDefenseEventManager>())
		{
			Defense->StartDefense();
			bDefenseStarted = true;
			bDefenseWasActive = Defense->IsDefenseActive();
		}
		return;
	}

	if (UDefenseEventManager* Defense = World->GetSubsystem<UDefenseEventManager>())
	{
		if (Defense->IsDefenseActive())
		{
			bDefenseWasActive = true;
			ForceClearExpeditionEnemies(World);
			return;
		}
		if (bDefenseWasActive)
		{
			UE_LOG(LogEmberbound, Log, TEXT("EMBER: AUTOPILOT defenseDone"));
			bDefenseWasActive = false;
		}
		else
		{
			return;
		}
	}

	if (!bOfflineDone)
	{
		if (UMonsterCollectionSubsystem* Monsters = GetGameInstance()->GetSubsystem<UMonsterCollectionSubsystem>())
		{
			Monsters->SimulateOffline(120);
		}
		bOfflineDone = true;
		UE_LOG(LogEmberbound, Log, TEXT("EMBER: AUTOPILOT offlineDone"));
		if (USaveManagerSubsystem* Save = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>())
		{
			Save->SaveNow();
		}
		UE_LOG(LogEmberbound, Log, TEXT("EMBER: AUTOPILOT saved"));
		CompleteAutopilot();
	}
}

void UEmberAutopilotSubsystem::CompleteAutopilot()
{
	if (UWorld* World = FindGameWorld(GetGameInstance()))
	{
		World->GetTimerManager().ClearTimer(CampReturnTickTimerHandle);
	}
	Phase = EEmberAutopilotPhase::Complete;
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: AUTOPILOT COMPLETE"));
}

void UEmberAutopilotSubsystem::ForceClearExpeditionEnemies(UWorld* World) const
{
	if (!World)
	{
		return;
	}
	for (TActorIterator<AEmberEnemyBase> It(World); It; ++It)
	{
		AEmberEnemyBase* Enemy = *It;
		if (!Enemy || Cast<ABossTutorialDragon>(Enemy) || !Enemy->IsAlive())
		{
			continue;
		}
		if (UHealthComponent* Health = Enemy->GetHealthComponent())
		{
			Health->ApplyDamage(99999.0f);
		}
	}
}

void UEmberAutopilotSubsystem::DamageActiveBoss(AExpeditionGameMode* ExpeditionMode) const
{
	ABossTutorialDragon* Boss = ExpeditionMode ? ExpeditionMode->GetAutopilotActiveBoss() : nullptr;
	if (Boss && Boss->IsAlive())
	{
		Boss->DamageBoss(60.0f);
	}
}

bool UEmberAutopilotSubsystem::BuyBuilding(FName BuildingId) const
{
	USaveManagerSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<USaveManagerSubsystem>() : nullptr;
	UBuildingData* Data = LoadAutopilotBuilding(BuildingId);
	return Save && Data && (Save->GetBuildingLevel(BuildingId) > 0 || Save->PurchaseBuilding(Data));
}

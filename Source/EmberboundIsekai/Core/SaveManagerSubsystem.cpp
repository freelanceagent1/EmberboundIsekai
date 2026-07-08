#include "Core/SaveManagerSubsystem.h"
#include "Core/EconomySubsystem.h"
#include "Core/EmberGameInstance.h"
#include "Core/EmberSaveGame.h"
#include "Core/SceneFlowSubsystem.h"
#include "Data/BuildingData.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "EmberboundIsekai.h"

void USaveManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<UEconomySubsystem>();
	LoadOrCreate();
	SyncEconomyFromSave();
	RegisterDebugHooks();
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: SaveLoaded %s"), *BuildSummary());
}

void USaveManagerSubsystem::Deinitialize()
{
	UnregisterDebugHooks();
	Super::Deinitialize();
}

bool USaveManagerSubsystem::CanAffordUpgrade(const UBuildingData* Data, int32 CurrentLevel, const UEconomySubsystem* Economy)
{
	if (!Data || !Economy || CurrentLevel >= Data->MaxLevel || !Data->UpgradeCosts.IsValidIndex(CurrentLevel - 1))
	{
		return false;
	}
	for (const TPair<EEmberResource, int32>& Pair : Data->UpgradeCosts[CurrentLevel - 1].Amounts)
	{
		if (Economy->Get(Pair.Key) < Pair.Value)
		{
			return false;
		}
	}
	return true;
}

bool USaveManagerSubsystem::SpendUpgradeCost(const UBuildingData* Data, int32 CurrentLevel, UEconomySubsystem* Economy)
{
	if (!CanAffordUpgrade(Data, CurrentLevel, Economy))
	{
		return false;
	}
	for (const TPair<EEmberResource, int32>& Pair : Data->UpgradeCosts[CurrentLevel - 1].Amounts)
	{
		Economy->Spend(Pair.Key, Pair.Value);
	}
	return true;
}

void USaveManagerSubsystem::SaveNow()
{
	if (!CurrentSave)
	{
		LoadOrCreate();
	}
	SyncSaveFromEconomy();
	CurrentSave->LastSaveUtc = FDateTime::UtcNow();
	UGameplayStatics::SaveGameToSlot(CurrentSave, SlotName, 0);
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: Saved"));
}

void USaveManagerSubsystem::ResetDebug()
{
	UGameplayStatics::DeleteGameInSlot(SlotName, 0);
	CurrentSave = Cast<UEmberSaveGame>(UGameplayStatics::CreateSaveGameObject(UEmberSaveGame::StaticClass()));
	SyncEconomyFromSave();
	SaveNow();
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: SaveReset"));
}

void USaveManagerSubsystem::GrantResources(int32 Gold, int32 Wood, int32 SoulAsh, int32 Essence, int32 Embers)
{
	if (UEconomySubsystem* Economy = GetEconomy())
	{
		Economy->Add(EEmberResource::Gold, Gold);
		Economy->Add(EEmberResource::Wood, Wood);
		Economy->Add(EEmberResource::SoulAsh, SoulAsh);
		Economy->Add(EEmberResource::MonsterEssence, Essence);
		Economy->Add(EEmberResource::Embers, Embers);
	}
	SaveNow();
}

int32 USaveManagerSubsystem::GetBuildingLevel(FName BuildingId) const
{
	return CurrentSave ? CurrentSave->BuildingLevels.FindRef(BuildingId) : 0;
}

bool USaveManagerSubsystem::PurchaseBuilding(UBuildingData* BuildingData)
{
	if (!CurrentSave || !BuildingData)
	{
		return false;
	}
	const int32 CurrentLevel = GetBuildingLevel(BuildingData->Id);
	const int32 SpendLevel = CurrentLevel <= 0 ? 1 : CurrentLevel;
	if (CurrentLevel >= BuildingData->MaxLevel || !SpendUpgradeCost(BuildingData, SpendLevel, GetEconomy()))
	{
		return false;
	}
	const int32 NewLevel = CurrentLevel + 1;
	CurrentSave->BuildingLevels.Add(BuildingData->Id, NewLevel);
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: BuildingUpgraded %s lvl=%d"), *BuildingData->Id.ToString(), NewLevel);
	SaveNow();
	return true;
}

float USaveManagerSubsystem::GetPlayerDamageMultiplier() const
{
	const int32 ForgeLevel = GetBuildingLevel(TEXT("grim-forge"));
	return 1.0f + 0.15f * static_cast<float>(ForgeLevel);
}

bool USaveManagerSubsystem::BankPendingRunResult()
{
	UEmberGameInstance* EmberGI = GetGameInstance() ? Cast<UEmberGameInstance>(GetGameInstance()) : nullptr;
	UEconomySubsystem* Economy = GetEconomy();
	if (!EmberGI || !Economy || !EmberGI->LastRunResult.bRunComplete || EmberGI->LastRunResult.bBankedToSave)
	{
		return false;
	}
	for (const TPair<EEmberResource, int32>& Pair : EmberGI->LastRunResult.BankedResources)
	{
		Economy->Add(Pair.Key, Pair.Value);
	}
	if (CurrentSave)
	{
		for (const TPair<FName, int32>& Pair : EmberGI->LastRunResult.MonsterShards)
		{
			CurrentSave->MonsterShards.FindOrAdd(Pair.Key) += Pair.Value;
		}
	}
	EmberGI->LastRunResult.bBankedToSave = true;
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: RunBanked Gold=%d Wood=%d SoulAsh=%d Embers=%d Shards=%d"),
		EmberGI->LastRunResult.BankedResources.FindRef(EEmberResource::Gold),
		EmberGI->LastRunResult.BankedResources.FindRef(EEmberResource::Wood),
		EmberGI->LastRunResult.BankedResources.FindRef(EEmberResource::SoulAsh),
		EmberGI->LastRunResult.BankedResources.FindRef(EEmberResource::Embers),
		EmberGI->LastRunResult.MonsterShards.FindRef(TEXT("TutorialDragon")));
	SaveNow();
	return true;
}

FString USaveManagerSubsystem::BuildSummary() const
{
	const int32 Gold = CurrentSave ? CurrentSave->Resources.FindRef(EEmberResource::Gold) : 0;
	const int32 Wood = CurrentSave ? CurrentSave->Resources.FindRef(EEmberResource::Wood) : 0;
	const int32 SoulAsh = CurrentSave ? CurrentSave->Resources.FindRef(EEmberResource::SoulAsh) : 0;
	const int32 Campfire = GetBuildingLevel(TEXT("campfire-core"));
	const int32 Forge = GetBuildingLevel(TEXT("grim-forge"));
	return FString::Printf(TEXT("Gold=%d Wood=%d SoulAsh=%d campfire=%d forge=%d"), Gold, Wood, SoulAsh, Campfire, Forge);
}

UEconomySubsystem* USaveManagerSubsystem::GetEconomy() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UEconomySubsystem>() : nullptr;
}

void USaveManagerSubsystem::LoadOrCreate()
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		CurrentSave = Cast<UEmberSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	}
	if (!CurrentSave)
	{
		CurrentSave = Cast<UEmberSaveGame>(UGameplayStatics::CreateSaveGameObject(UEmberSaveGame::StaticClass()));
	}
}

void USaveManagerSubsystem::SyncEconomyFromSave()
{
	if (!CurrentSave)
	{
		return;
	}
	if (UEconomySubsystem* Economy = GetEconomy())
	{
		for (EEmberResource Resource : { EEmberResource::Gold, EEmberResource::Wood, EEmberResource::SoulAsh, EEmberResource::MonsterEssence, EEmberResource::Embers })
		{
			Economy->Set(Resource, CurrentSave->Resources.FindRef(Resource));
		}
	}
}

void USaveManagerSubsystem::SyncSaveFromEconomy()
{
	if (!CurrentSave)
	{
		return;
	}
	if (UEconomySubsystem* Economy = GetEconomy())
	{
		CurrentSave->Resources = Economy->GetWallet();
	}
}

void USaveManagerSubsystem::RegisterDebugHooks()
{
	IConsoleManager& Console = IConsoleManager::Get();
	Console.RegisterConsoleCommand(TEXT("Ember.GrantResources"), TEXT("Grant camp resources."), FConsoleCommandWithArgsDelegate::CreateUObject(this, &USaveManagerSubsystem::GrantResourcesConsole), ECVF_Default);
	Console.RegisterConsoleCommand(TEXT("Ember.ResetSave"), TEXT("Reset Ember save slot."), FConsoleCommandWithArgsDelegate::CreateUObject(this, &USaveManagerSubsystem::ResetSaveConsole), ECVF_Default);
	Console.RegisterConsoleCommand(TEXT("Ember.TravelExpedition"), TEXT("Travel to expedition."), FConsoleCommandWithArgsDelegate::CreateUObject(this, &USaveManagerSubsystem::TravelExpeditionConsole), ECVF_Default);
	Console.RegisterConsoleCommand(TEXT("Ember.TravelCamp"), TEXT("Travel to camp."), FConsoleCommandWithArgsDelegate::CreateUObject(this, &USaveManagerSubsystem::TravelCampConsole), ECVF_Default);
	Console.RegisterConsoleCommand(TEXT("Ember.BuyBuilding"), TEXT("Debug buy building by id."), FConsoleCommandWithArgsDelegate::CreateUObject(this, &USaveManagerSubsystem::BuyBuildingConsole), ECVF_Default);
}

void USaveManagerSubsystem::UnregisterDebugHooks()
{
	IConsoleManager& Console = IConsoleManager::Get();
	Console.UnregisterConsoleObject(TEXT("Ember.GrantResources"), false);
	Console.UnregisterConsoleObject(TEXT("Ember.ResetSave"), false);
	Console.UnregisterConsoleObject(TEXT("Ember.TravelExpedition"), false);
	Console.UnregisterConsoleObject(TEXT("Ember.TravelCamp"), false);
	Console.UnregisterConsoleObject(TEXT("Ember.BuyBuilding"), false);
}

void USaveManagerSubsystem::GrantResourcesConsole(const TArray<FString>& Args)
{
	GrantResources(
		Args.IsValidIndex(0) ? FCString::Atoi(*Args[0]) : 0,
		Args.IsValidIndex(1) ? FCString::Atoi(*Args[1]) : 0,
		Args.IsValidIndex(2) ? FCString::Atoi(*Args[2]) : 0,
		Args.IsValidIndex(3) ? FCString::Atoi(*Args[3]) : 0,
		Args.IsValidIndex(4) ? FCString::Atoi(*Args[4]) : 0);
}

void USaveManagerSubsystem::ResetSaveConsole(const TArray<FString>&)
{
	ResetDebug();
}

void USaveManagerSubsystem::TravelExpeditionConsole(const TArray<FString>&)
{
	if (USceneFlowSubsystem* Flow = GetGameInstance() ? GetGameInstance()->GetSubsystem<USceneFlowSubsystem>() : nullptr)
	{
		Flow->TravelToExpedition();
	}
}

void USaveManagerSubsystem::TravelCampConsole(const TArray<FString>&)
{
	if (USceneFlowSubsystem* Flow = GetGameInstance() ? GetGameInstance()->GetSubsystem<USceneFlowSubsystem>() : nullptr)
	{
		Flow->TravelToCamp();
	}
}

void USaveManagerSubsystem::BuyBuildingConsole(const TArray<FString>& Args)
{
	if (!Args.IsValidIndex(0))
	{
		return;
	}
	PurchaseBuilding(LoadBuildingData(FName(*Args[0])));
}

UBuildingData* USaveManagerSubsystem::LoadBuildingData(FName BuildingId) const
{
	const TMap<FName, FString> AssetNames = {
		{ TEXT("campfire-core"), TEXT("DA_CampfireCore") },
		{ TEXT("grim-forge"), TEXT("DA_GrimForge") },
		{ TEXT("monster-pen"), TEXT("DA_MonsterPen") },
		{ TEXT("watchtower"), TEXT("DA_Watchtower") }
	};
	const FString AssetName = AssetNames.Contains(BuildingId) ? AssetNames[BuildingId] : BuildingId.ToString();
	const FString Path = FString::Printf(TEXT("/Game/Emberbound/Data/Buildings/%s.%s"), *AssetName, *AssetName);
	return LoadObject<UBuildingData>(nullptr, *Path);
}

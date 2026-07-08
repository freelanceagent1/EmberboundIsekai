#include "Core/MonsterCollectionSubsystem.h"
#include "Core/EconomySubsystem.h"
#include "Core/EmberSaveGame.h"
#include "Core/SaveManagerSubsystem.h"
#include "Core/TitleSubsystem.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Engine/AssetManager.h"
#include "Engine/GameInstance.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "UI/MonsterUnlockToastWidget.h"
#include "UI/OfflineRewardWidget.h"
#include "TimerManager.h"
#include "EmberboundIsekai.h"

namespace
{
	const TCHAR* MonsterDataPath = TEXT("/Game/Emberbound/Data/Monsters");
}

void UMonsterCollectionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<USaveManagerSubsystem>();
	LoadCatalog();
	RegisterDebugHooks();
}

void UMonsterCollectionSubsystem::Deinitialize()
{
	UnregisterDebugHooks();
	Super::Deinitialize();
}

void UMonsterCollectionSubsystem::LoadCatalog()
{
	Catalog.Reset();
	const TArray<FString> AssetNames = { TEXT("DA_Slime"), TEXT("DA_BoneRat"), TEXT("DA_SkeletonArcher") };
	for (const FString& AssetName : AssetNames)
	{
		const FString Path = FString::Printf(TEXT("%s/%s.%s"), MonsterDataPath, *AssetName, *AssetName);
		if (UMonsterData* Data = LoadObject<UMonsterData>(nullptr, *Path))
		{
			Catalog.Add(Data);
		}
	}
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: MonsterCatalogLoaded count=%d"), Catalog.Num());
}

UMonsterData* UMonsterCollectionSubsystem::FindMonster(FName MonsterId) const
{
	for (UMonsterData* Data : Catalog)
	{
		if (Data && Data->Id == MonsterId)
		{
			return Data;
		}
	}
	return nullptr;
}

int32 UMonsterCollectionSubsystem::GetShardCount(FName MonsterId) const
{
	const UEmberSaveGame* Save = GetSave();
	return Save ? Save->MonsterShards.FindRef(MonsterId) : 0;
}

bool UMonsterCollectionSubsystem::IsOwned(FName MonsterId) const
{
	const UEmberSaveGame* Save = GetSave();
	return Save && Save->OwnedMonsters.Contains(MonsterId);
}

EMonsterAssignmentRole UMonsterCollectionSubsystem::GetAssignment(FName MonsterId) const
{
	const UEmberSaveGame* Save = GetSave();
	if (!Save)
	{
		return EMonsterAssignmentRole::None;
	}
	for (const FMonsterAssignmentSave& Assignment : Save->Assignments)
	{
		if (Assignment.MonsterId == MonsterId)
		{
			return RoleFromName(Assignment.AssignmentId.ToString());
		}
	}
	return EMonsterAssignmentRole::None;
}

FName UMonsterCollectionSubsystem::GetAssignedMonster(EMonsterAssignmentRole Role) const
{
	const UEmberSaveGame* Save = GetSave();
	const FName RoleName = RoleToName(Role);
	if (!Save || Role == EMonsterAssignmentRole::None)
	{
		return NAME_None;
	}
	for (const FMonsterAssignmentSave& Assignment : Save->Assignments)
	{
		if (Assignment.AssignmentId == RoleName)
		{
			return Assignment.MonsterId;
		}
	}
	return NAME_None;
}

int32 UMonsterCollectionSubsystem::GetAssignmentSlotCap() const
{
	const USaveManagerSubsystem* SaveManager = GetSaveManager();
	return SaveManager ? SaveManager->GetBuildingLevel(TEXT("monster-pen")) + 1 : 1;
}

int32 UMonsterCollectionSubsystem::GetUsedAssignmentSlots() const
{
	const UEmberSaveGame* Save = GetSave();
	return Save ? Save->Assignments.Num() : 0;
}

float UMonsterCollectionSubsystem::GetDefenderPower() const
{
	float Total = 0.0f;
	const UEmberSaveGame* Save = GetSave();
	if (!Save)
	{
		return Total;
	}
	for (const FMonsterAssignmentSave& Assignment : Save->Assignments)
	{
		if (Assignment.AssignmentId == RoleToName(EMonsterAssignmentRole::Defender))
		{
			if (const UMonsterData* Data = FindMonster(Assignment.MonsterId))
			{
				Total += Data->DefenderDps;
			}
		}
	}
	return Total;
}

void UMonsterCollectionSubsystem::GrantShards(FName MonsterId, int32 Amount)
{
	UEmberSaveGame* Save = GetSave();
	if (!Save || Amount <= 0)
	{
		return;
	}
	Save->MonsterShards.FindOrAdd(MonsterId) += Amount;
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: ShardsGranted %s amount=%d total=%d"), *MonsterId.ToString(), Amount, Save->MonsterShards.FindRef(MonsterId));
	CheckUnlock(MonsterId);
	SaveIfAvailable();
}

bool UMonsterCollectionSubsystem::AssignMonster(FName MonsterId, EMonsterAssignmentRole Role)
{
	UEmberSaveGame* Save = GetSave();
	UMonsterData* Data = FindMonster(MonsterId);
	if (!Save || !Data || !IsOwned(MonsterId) || !Data->AllowsRole(Role))
	{
		return false;
	}

	for (int32 Index = Save->Assignments.Num() - 1; Index >= 0; --Index)
	{
		if (Save->Assignments[Index].MonsterId == MonsterId || (Role != EMonsterAssignmentRole::None && Save->Assignments[Index].AssignmentId == RoleToName(Role)))
		{
			Save->Assignments.RemoveAt(Index);
		}
	}
	if (Role != EMonsterAssignmentRole::None)
	{
		if (Save->Assignments.Num() >= GetAssignmentSlotCap())
		{
			UE_LOG(LogEmberbound, Warning, TEXT("EMBER: AssignmentRejected %s role=%s reason=slot-cap cap=%d"), *MonsterId.ToString(), *RoleToName(Role).ToString(), GetAssignmentSlotCap());
			SaveIfAvailable();
			return false;
		}
		FMonsterAssignmentSave NewAssignment;
		NewAssignment.MonsterId = MonsterId;
		NewAssignment.AssignmentId = RoleToName(Role);
		Save->Assignments.Add(NewAssignment);
	}
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: MonsterAssigned %s role=%s slots=%d/%d defender=%.1f"), *MonsterId.ToString(), *RoleToName(Role).ToString(), Save->Assignments.Num(), GetAssignmentSlotCap(), GetDefenderPower());
	SaveIfAvailable();
	return true;
}

FOfflineRewardResult UMonsterCollectionSubsystem::ApplyOfflineIncome()
{
	FOfflineRewardResult Result;
	UEmberSaveGame* Save = GetSave();
	UEconomySubsystem* Economy = GetEconomy();
	USaveManagerSubsystem* SaveManager = GetSaveManager();
	if (!Save || !Economy || !SaveManager || !Save->LastSaveUtc.GetTicks())
	{
		return Result;
	}
	const FTimespan Elapsed = FDateTime::UtcNow() - Save->LastSaveUtc;
	const int32 ElapsedMinutes = FMath::Max(0, FMath::FloorToInt(Elapsed.GetTotalMinutes()));
	const int32 CapMinutes = FMath::Max(1, SaveManager->GetBuildingLevel(TEXT("campfire-core"))) * 240;
	Result.Minutes = FMath::Min(ElapsedMinutes, CapMinutes);
	Result.bCapped = ElapsedMinutes > CapMinutes;
	if (Result.Minutes <= 0)
	{
		return Result;
	}

	for (const FMonsterAssignmentSave& Assignment : Save->Assignments)
	{
		if (Assignment.AssignmentId != RoleToName(EMonsterAssignmentRole::Worker))
		{
			continue;
		}
		if (const UMonsterData* Data = FindMonster(Assignment.MonsterId))
		{
			for (const FMonsterWorkerOutput& Output : Data->WorkerOutputs)
			{
				const int32 Amount = FMath::FloorToInt(Output.PerMinute * static_cast<float>(Result.Minutes));
				if (Amount > 0)
				{
					const UTitleSubsystem* Titles = GetGameInstance() ? GetGameInstance()->GetSubsystem<UTitleSubsystem>() : nullptr;
					Result.Rewards.FindOrAdd(Output.Resource) += Titles ? Titles->ApplyOfflineProduction(Amount) : Amount;
				}
			}
		}
	}
	for (const TPair<EEmberResource, int32>& Pair : Result.Rewards)
	{
		Economy->Add(Pair.Key, Pair.Value);
	}
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: OfflineReward mins=%d capped=%d gold=%d soulash=%d"), Result.Minutes, Result.bCapped ? 1 : 0, Result.Rewards.FindRef(EEmberResource::Gold), Result.Rewards.FindRef(EEmberResource::SoulAsh));
	if (UTitleSubsystem* Titles = GetGameInstance() ? GetGameInstance()->GetSubsystem<UTitleSubsystem>() : nullptr)
	{
		Titles->NotifyOfflineRewardCollected();
	}
	SaveManager->SaveNow();
	return Result;
}

FOfflineRewardResult UMonsterCollectionSubsystem::SimulateOffline(int32 Minutes)
{
	UEmberSaveGame* Save = GetSave();
	if (Save)
	{
		Save->LastSaveUtc = FDateTime::UtcNow() - FTimespan::FromMinutes(FMath::Max(0, Minutes));
	}
	FOfflineRewardResult Reward = ApplyOfflineIncome();
	ShowOfflineRewardPopup(Reward);
	return Reward;
}

void UMonsterCollectionSubsystem::CheckUnlock(FName MonsterId)
{
	UEmberSaveGame* Save = GetSave();
	UMonsterData* Data = FindMonster(MonsterId);
	if (!Save || !Data || Save->OwnedMonsters.Contains(MonsterId) || Save->MonsterShards.FindRef(MonsterId) < Data->ShardsToUnlock)
	{
		return;
	}
	Save->OwnedMonsters.Add(MonsterId);
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: MonsterUnlocked %s"), *MonsterId.ToString());
	ShowUnlockToast(MonsterId);
}

void UMonsterCollectionSubsystem::ShowUnlockToast(FName MonsterId)
{
	UWorld* World = GetWorld();
	APlayerController* PC = World ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;
	if (!PC)
	{
		return;
	}
	UMonsterUnlockToastWidget* Toast = CreateWidget<UMonsterUnlockToastWidget>(PC, UMonsterUnlockToastWidget::StaticClass());
	if (!Toast)
	{
		return;
	}
	Toast->SetMonsterId(MonsterId);
	Toast->AddToViewport(180);
	FTimerHandle RemoveHandle;
	World->GetTimerManager().SetTimer(RemoveHandle, [Toast]()
	{
		if (Toast)
		{
			Toast->RemoveFromParent();
		}
	}, 3.0f, false);
}

void UMonsterCollectionSubsystem::ShowOfflineRewardPopup(const FOfflineRewardResult& Reward)
{
	if (Reward.Minutes <= 0 || Reward.Rewards.Num() == 0)
	{
		return;
	}
	UWorld* World = GetWorld();
	APlayerController* PC = World ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;
	if (!PC)
	{
		return;
	}
	UOfflineRewardWidget* RewardWidget = CreateWidget<UOfflineRewardWidget>(PC, UOfflineRewardWidget::StaticClass());
	if (RewardWidget)
	{
		RewardWidget->SetReward(Reward);
		RewardWidget->AddToViewport(160);
	}
}

void UMonsterCollectionSubsystem::SaveIfAvailable()
{
	if (USaveManagerSubsystem* SaveManager = GetSaveManager())
	{
		SaveManager->SaveNow();
	}
}

UEmberSaveGame* UMonsterCollectionSubsystem::GetSave() const
{
	const USaveManagerSubsystem* SaveManager = GetSaveManager();
	return SaveManager ? SaveManager->GetCurrentSave() : nullptr;
}

UEconomySubsystem* UMonsterCollectionSubsystem::GetEconomy() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UEconomySubsystem>() : nullptr;
}

USaveManagerSubsystem* UMonsterCollectionSubsystem::GetSaveManager() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<USaveManagerSubsystem>() : nullptr;
}

void UMonsterCollectionSubsystem::RegisterDebugHooks()
{
	IConsoleManager& Console = IConsoleManager::Get();
	Console.RegisterConsoleCommand(TEXT("Ember.GrantShards"), TEXT("Grant typed monster shards. Usage: Ember.GrantShards slime 5"), FConsoleCommandWithArgsDelegate::CreateUObject(this, &UMonsterCollectionSubsystem::GrantShardsConsole), ECVF_Default);
	Console.RegisterConsoleCommand(TEXT("Ember.AssignMonster"), TEXT("Assign monster role. Usage: Ember.AssignMonster slime worker"), FConsoleCommandWithArgsDelegate::CreateUObject(this, &UMonsterCollectionSubsystem::AssignMonsterConsole), ECVF_Default);
	Console.RegisterConsoleCommand(TEXT("Ember.SimulateOffline"), TEXT("Rewind save timestamp and apply offline income. Usage: Ember.SimulateOffline 120"), FConsoleCommandWithArgsDelegate::CreateUObject(this, &UMonsterCollectionSubsystem::SimulateOfflineConsole), ECVF_Default);
}

void UMonsterCollectionSubsystem::UnregisterDebugHooks()
{
	IConsoleManager& Console = IConsoleManager::Get();
	Console.UnregisterConsoleObject(TEXT("Ember.GrantShards"), false);
	Console.UnregisterConsoleObject(TEXT("Ember.AssignMonster"), false);
	Console.UnregisterConsoleObject(TEXT("Ember.SimulateOffline"), false);
}

void UMonsterCollectionSubsystem::GrantShardsConsole(const TArray<FString>& Args)
{
	if (Args.Num() >= 2)
	{
		GrantShards(FName(*Args[0]), FCString::Atoi(*Args[1]));
	}
}

void UMonsterCollectionSubsystem::AssignMonsterConsole(const TArray<FString>& Args)
{
	if (Args.Num() >= 2)
	{
		AssignMonster(FName(*Args[0]), RoleFromName(Args[1]));
	}
}

void UMonsterCollectionSubsystem::SimulateOfflineConsole(const TArray<FString>& Args)
{
	SimulateOffline(Args.Num() > 0 ? FCString::Atoi(*Args[0]) : 0);
}

EMonsterAssignmentRole UMonsterCollectionSubsystem::RoleFromName(const FString& RoleName)
{
	if (RoleName.Equals(TEXT("companion"), ESearchCase::IgnoreCase)) { return EMonsterAssignmentRole::Companion; }
	if (RoleName.Equals(TEXT("worker"), ESearchCase::IgnoreCase)) { return EMonsterAssignmentRole::Worker; }
	if (RoleName.Equals(TEXT("defender"), ESearchCase::IgnoreCase)) { return EMonsterAssignmentRole::Defender; }
	return EMonsterAssignmentRole::None;
}

FName UMonsterCollectionSubsystem::RoleToName(EMonsterAssignmentRole Role)
{
	switch (Role)
	{
	case EMonsterAssignmentRole::Companion: return TEXT("companion");
	case EMonsterAssignmentRole::Worker: return TEXT("worker");
	case EMonsterAssignmentRole::Defender: return TEXT("defender");
	default: return TEXT("none");
	}
}

FString UMonsterCollectionSubsystem::ResourceLogName(EEmberResource Resource)
{
	switch (Resource)
	{
	case EEmberResource::Gold: return TEXT("gold");
	case EEmberResource::SoulAsh: return TEXT("soulash");
	case EEmberResource::Wood: return TEXT("wood");
	case EEmberResource::MonsterEssence: return TEXT("essence");
	case EEmberResource::Embers: return TEXT("embers");
	default: return TEXT("resource");
	}
}

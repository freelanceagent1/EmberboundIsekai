#include "Core/TitleSubsystem.h"
#include "Core/EmberSaveGame.h"
#include "Core/SaveManagerSubsystem.h"
#include "UI/TitleUnlockToastWidget.h"
#include "Engine/GameInstance.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "EmberboundIsekai.h"

namespace
{
	const TCHAR* TitleDataPath = TEXT("/Game/Emberbound/Data/Titles");

	int32 ApplyPercentToInt(int32 Amount, float Percent)
	{
		return FMath::Max(0, FMath::RoundToInt(static_cast<float>(Amount) * (1.0f + Percent)));
	}
}

void UTitleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<USaveManagerSubsystem>();
	LoadCatalog();
	RegisterDebugHooks();
}

void UTitleSubsystem::Deinitialize()
{
	UnregisterDebugHooks();
	Super::Deinitialize();
}

void UTitleSubsystem::LoadCatalog()
{
	Catalog.Reset();
	const TArray<FString> AssetNames = {
		TEXT("DA_Title_FormerOfficeDrone"),
		TEXT("DA_Title_TruckKunSurvivor"),
		TEXT("DA_Title_FailedChosenOne"),
		TEXT("DA_Title_SpreadsheetHero"),
		TEXT("DA_Title_GenreSavvyIdiot")
	};
	for (const FString& AssetName : AssetNames)
	{
		const FString Path = FString::Printf(TEXT("%s/%s.%s"), TitleDataPath, *AssetName, *AssetName);
		if (UTitleData* Data = LoadObject<UTitleData>(nullptr, *Path))
		{
			Catalog.Add(Data);
		}
	}
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: TitleCatalogLoaded count=%d"), Catalog.Num());
}

bool UTitleSubsystem::IsUnlocked(FName TitleId) const
{
	const UEmberSaveGame* Save = GetSave();
	return Save && Save->UnlockedTitles.Contains(TitleId);
}

void UTitleSubsystem::NotifyOfflineRewardCollected()
{
	if (UEmberSaveGame* Save = GetSave())
	{
		Save->TitleCounters.FindOrAdd(CounterKeyForCondition(ETitleUnlockCondition::OfflineRewardCollected)) = 1;
		CheckUnlocks(ETitleUnlockCondition::OfflineRewardCollected, 1);
	}
}

void UTitleSubsystem::NotifyBossDefeated()
{
	if (UEmberSaveGame* Save = GetSave())
	{
		const FName CounterKey = CounterKeyForCondition(ETitleUnlockCondition::BossDefeated);
		const int32 Counter = ++Save->TitleCounters.FindOrAdd(CounterKey);
		CheckUnlocks(ETitleUnlockCondition::BossDefeated, Counter);
	}
}

void UTitleSubsystem::NotifyBuildingUpgraded()
{
	if (UEmberSaveGame* Save = GetSave())
	{
		const FName CounterKey = CounterKeyForCondition(ETitleUnlockCondition::BuildingUpgrades);
		const int32 Counter = ++Save->TitleCounters.FindOrAdd(CounterKey);
		CheckUnlocks(ETitleUnlockCondition::BuildingUpgrades, Counter);
	}
}

void UTitleSubsystem::NotifyRunUpgradePicked()
{
	if (UEmberSaveGame* Save = GetSave())
	{
		const FName CounterKey = CounterKeyForCondition(ETitleUnlockCondition::RunUpgradesPicked);
		const int32 Counter = ++Save->TitleCounters.FindOrAdd(CounterKey);
		CheckUnlocks(ETitleUnlockCondition::RunUpgradesPicked, Counter);
	}
}

void UTitleSubsystem::UnlockTitle(FName TitleId, bool bAllowDebugOnly)
{
	UEmberSaveGame* Save = GetSave();
	if (!Save || TitleId.IsNone() || Save->UnlockedTitles.Contains(TitleId))
	{
		return;
	}
	UTitleData* FoundTitle = nullptr;
	for (UTitleData* Title : Catalog)
	{
		if (Title && Title->Id == TitleId)
		{
			FoundTitle = Title;
			break;
		}
	}
	if (FoundTitle && FoundTitle->bDebugOnly && !bAllowDebugOnly)
	{
		return;
	}
	Save->UnlockedTitles.Add(TitleId);
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: TitleUnlocked %s"), *TitleId.ToString());
	ShowTitleToast(TitleId);
	SaveIfAvailable();
}

float UTitleSubsystem::GetPassiveMagnitude(ETitlePassiveEffectType EffectType) const
{
	float Magnitude = 0.0f;
	for (const FTitlePassiveSpec& Passive : GetUnlockedPassives())
	{
		if (Passive.EffectType == EffectType)
		{
			Magnitude += Passive.Magnitude;
		}
	}
	return Magnitude;
}

TArray<FTitlePassiveSpec> UTitleSubsystem::GetUnlockedPassives() const
{
	TArray<FTitlePassiveSpec> Passives;
	const UEmberSaveGame* Save = GetSave();
	if (!Save)
	{
		return Passives;
	}
	for (UTitleData* Title : Catalog)
	{
		if (Title && Save->UnlockedTitles.Contains(Title->Id))
		{
			Passives.Add(Title->Passive);
		}
	}
	return Passives;
}

int32 UTitleSubsystem::ApplyOfflineProduction(int32 Amount) const
{
	return ApplyOfflineProductionPassive(Amount, GetUnlockedPassives());
}

int32 UTitleSubsystem::ApplyUpgradeCost(int32 Amount) const
{
	return ApplyUpgradeCostPassive(Amount, GetUnlockedPassives());
}

int32 UTitleSubsystem::ApplyXpGain(int32 Amount) const
{
	return ApplyXpPassive(Amount, GetUnlockedPassives());
}

float UTitleSubsystem::ApplyPlayerDamage(float Damage, float HealthFraction) const
{
	return ApplyDamagePassive(Damage, HealthFraction, GetUnlockedPassives());
}

int32 UTitleSubsystem::ApplyOfflineProductionPassive(int32 Amount, const TArray<FTitlePassiveSpec>& Passives)
{
	float Percent = 0.0f;
	for (const FTitlePassiveSpec& Passive : Passives)
	{
		if (Passive.EffectType == ETitlePassiveEffectType::OfflineProductionPercent)
		{
			Percent += Passive.Magnitude;
		}
	}
	return ApplyPercentToInt(Amount, Percent);
}

int32 UTitleSubsystem::ApplyUpgradeCostPassive(int32 Amount, const TArray<FTitlePassiveSpec>& Passives)
{
	float Percent = 0.0f;
	for (const FTitlePassiveSpec& Passive : Passives)
	{
		if (Passive.EffectType == ETitlePassiveEffectType::UpgradeCostPercent)
		{
			Percent += Passive.Magnitude;
		}
	}
	return ApplyPercentToInt(Amount, Percent);
}

int32 UTitleSubsystem::ApplyXpPassive(int32 Amount, const TArray<FTitlePassiveSpec>& Passives)
{
	float Percent = 0.0f;
	for (const FTitlePassiveSpec& Passive : Passives)
	{
		if (Passive.EffectType == ETitlePassiveEffectType::XpGainPercent)
		{
			Percent += Passive.Magnitude;
		}
	}
	return ApplyPercentToInt(Amount, Percent);
}

float UTitleSubsystem::ApplyDamagePassive(float Damage, float HealthFraction, const TArray<FTitlePassiveSpec>& Passives)
{
	float Percent = 0.0f;
	if (HealthFraction < 0.30f)
	{
		for (const FTitlePassiveSpec& Passive : Passives)
		{
			if (Passive.EffectType == ETitlePassiveEffectType::LowHealthDamagePercent)
			{
				Percent += Passive.Magnitude;
			}
		}
	}
	return Damage * (1.0f + Percent);
}

bool UTitleSubsystem::DoesCounterSatisfyUnlock(const UTitleData& Title, int32 CounterValue)
{
	if (Title.UnlockCondition == ETitleUnlockCondition::None)
	{
		return false;
	}
	return CounterValue >= FMath::Max(1, Title.UnlockParam);
}

void UTitleSubsystem::CheckUnlocks(ETitleUnlockCondition Condition, int32 CounterValue)
{
	for (UTitleData* Title : Catalog)
	{
		if (Title && Title->UnlockCondition == Condition && DoesCounterSatisfyUnlock(*Title, CounterValue))
		{
			UnlockTitle(Title->Id);
		}
	}
}

void UTitleSubsystem::ShowTitleToast(FName TitleId)
{
	UWorld* World = GetWorld();
	APlayerController* PC = World ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;
	if (!PC)
	{
		return;
	}
	UTitleUnlockToastWidget* Toast = CreateWidget<UTitleUnlockToastWidget>(PC, UTitleUnlockToastWidget::StaticClass());
	if (!Toast)
	{
		return;
	}
	Toast->SetTitleId(TitleId);
	Toast->AddToViewport(185);
	TWeakObjectPtr<UTitleUnlockToastWidget> WeakToast = Toast;
	FTimerHandle RemoveHandle;
	World->GetTimerManager().SetTimer(RemoveHandle, [WeakToast]()
	{
		if (UTitleUnlockToastWidget* Toast = WeakToast.Get())
		{
			Toast->RemoveFromParent();
		}
	}, 3.0f, false);
}

UEmberSaveGame* UTitleSubsystem::GetSave() const
{
	const USaveManagerSubsystem* SaveManager = GetSaveManager();
	return SaveManager ? SaveManager->GetCurrentSave() : nullptr;
}

USaveManagerSubsystem* UTitleSubsystem::GetSaveManager() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<USaveManagerSubsystem>() : nullptr;
}

void UTitleSubsystem::SaveIfAvailable()
{
	if (USaveManagerSubsystem* SaveManager = GetSaveManager())
	{
		SaveManager->SaveNow();
	}
}

void UTitleSubsystem::RegisterDebugHooks()
{
	IConsoleManager& Console = IConsoleManager::Get();
	Console.RegisterConsoleCommand(TEXT("Ember.GrantTitle"), TEXT("Debug unlock title by id."), FConsoleCommandWithArgsDelegate::CreateUObject(this, &UTitleSubsystem::GrantTitleConsole), ECVF_Default);
	Console.RegisterConsoleCommand(TEXT("Ember.DebugTitleCounter"), TEXT("Set a title counter. Usage: Ember.DebugTitleCounter building 5"), FConsoleCommandWithArgsDelegate::CreateUObject(this, &UTitleSubsystem::DebugCounterConsole), ECVF_Default);
	Console.RegisterConsoleCommand(TEXT("Ember.DebugTitleEffects"), TEXT("Log current title passive calculations."), FConsoleCommandWithArgsDelegate::CreateUObject(this, &UTitleSubsystem::DebugEffectsConsole), ECVF_Default);
}

void UTitleSubsystem::UnregisterDebugHooks()
{
	IConsoleManager& Console = IConsoleManager::Get();
	Console.UnregisterConsoleObject(TEXT("Ember.GrantTitle"), false);
	Console.UnregisterConsoleObject(TEXT("Ember.DebugTitleCounter"), false);
	Console.UnregisterConsoleObject(TEXT("Ember.DebugTitleEffects"), false);
}

void UTitleSubsystem::DebugEffectsConsole(const TArray<FString>&)
{
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: TitleEffects offline120=%d cost20=%d xp20=%d damageLow=%.2f damageHigh=%.2f"),
		ApplyOfflineProduction(120),
		ApplyUpgradeCost(20),
		ApplyXpGain(20),
		ApplyPlayerDamage(12.0f, 0.29f),
		ApplyPlayerDamage(12.0f, 0.30f));
}

void UTitleSubsystem::GrantTitleConsole(const TArray<FString>& Args)
{
	if (Args.IsValidIndex(0))
	{
		UnlockTitle(FName(*Args[0]), true);
	}
}

void UTitleSubsystem::DebugCounterConsole(const TArray<FString>& Args)
{
	if (!Args.IsValidIndex(0))
	{
		return;
	}
	const int32 Value = Args.IsValidIndex(1) ? FCString::Atoi(*Args[1]) : 1;
	const FString Counter = Args[0].ToLower();
	ETitleUnlockCondition Condition = ETitleUnlockCondition::None;
	if (Counter == TEXT("offline")) { Condition = ETitleUnlockCondition::OfflineRewardCollected; }
	else if (Counter == TEXT("boss")) { Condition = ETitleUnlockCondition::BossDefeated; }
	else if (Counter == TEXT("building")) { Condition = ETitleUnlockCondition::BuildingUpgrades; }
	else if (Counter == TEXT("upgrade")) { Condition = ETitleUnlockCondition::RunUpgradesPicked; }
	if (UEmberSaveGame* Save = GetSave())
	{
		Save->TitleCounters.FindOrAdd(CounterKeyForCondition(Condition)) = Value;
		CheckUnlocks(Condition, Value);
		SaveIfAvailable();
	}
}

FName UTitleSubsystem::CounterKeyForCondition(ETitleUnlockCondition Condition)
{
	switch (Condition)
	{
	case ETitleUnlockCondition::OfflineRewardCollected: return TEXT("offline-rewards");
	case ETitleUnlockCondition::BossDefeated: return TEXT("boss-defeats");
	case ETitleUnlockCondition::BuildingUpgrades: return TEXT("building-upgrades");
	case ETitleUnlockCondition::RunUpgradesPicked: return TEXT("run-upgrades-picked");
	default: return NAME_None;
	}
}

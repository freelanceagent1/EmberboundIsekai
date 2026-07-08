#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/TitleData.h"
#include "TitleSubsystem.generated.h"

class UEmberSaveGame;
class USaveManagerSubsystem;

UCLASS()
class EMBERBOUNDISEKAI_API UTitleSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	const TArray<UTitleData*>& GetCatalog() const { return Catalog; }
	bool IsUnlocked(FName TitleId) const;
	void NotifyOfflineRewardCollected();
	void NotifyBossDefeated();
	void NotifyBuildingUpgraded();
	void NotifyRunUpgradePicked();
	void UnlockTitle(FName TitleId, bool bAllowDebugOnly = false);

	float GetPassiveMagnitude(ETitlePassiveEffectType EffectType) const;
	TArray<FTitlePassiveSpec> GetUnlockedPassives() const;

	int32 ApplyOfflineProduction(int32 Amount) const;
	int32 ApplyUpgradeCost(int32 Amount) const;
	int32 ApplyXpGain(int32 Amount) const;
	float ApplyPlayerDamage(float Damage, float HealthFraction) const;

	static int32 ApplyOfflineProductionPassive(int32 Amount, const TArray<FTitlePassiveSpec>& Passives);
	static int32 ApplyUpgradeCostPassive(int32 Amount, const TArray<FTitlePassiveSpec>& Passives);
	static int32 ApplyXpPassive(int32 Amount, const TArray<FTitlePassiveSpec>& Passives);
	static float ApplyDamagePassive(float Damage, float HealthFraction, const TArray<FTitlePassiveSpec>& Passives);
	static bool DoesCounterSatisfyUnlock(const UTitleData& Title, int32 CounterValue);

private:
	UPROPERTY()
	TArray<TObjectPtr<UTitleData>> Catalog;

	FDelegateHandle GrantTitleCommandHandle;
	FDelegateHandle DebugCounterCommandHandle;
	FDelegateHandle DebugEffectsCommandHandle;

	void LoadCatalog();
	void CheckUnlocks(ETitleUnlockCondition Condition, int32 CounterValue);
	void ShowTitleToast(FName TitleId);
	UEmberSaveGame* GetSave() const;
	USaveManagerSubsystem* GetSaveManager() const;
	void SaveIfAvailable();
	void RegisterDebugHooks();
	void UnregisterDebugHooks();
	void GrantTitleConsole(const TArray<FString>& Args);
	void DebugCounterConsole(const TArray<FString>& Args);
	void DebugEffectsConsole(const TArray<FString>& Args);
	static FName CounterKeyForCondition(ETitleUnlockCondition Condition);
};

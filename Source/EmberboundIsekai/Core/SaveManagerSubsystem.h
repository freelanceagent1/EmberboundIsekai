#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/ResourceTypes.h"
#include "SaveManagerSubsystem.generated.h"

class UBuildingData;
class UEconomySubsystem;
class UEmberSaveGame;
struct FEmberRunResult;

UCLASS()
class EMBERBOUNDISEKAI_API USaveManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static constexpr const TCHAR* SlotName = TEXT("EmberboundSave");

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	static bool CanAffordUpgrade(const UBuildingData* Data, int32 CurrentLevel, const UEconomySubsystem* Economy);
	static bool SpendUpgradeCost(const UBuildingData* Data, int32 CurrentLevel, UEconomySubsystem* Economy);

	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveNow();

	UFUNCTION(BlueprintCallable, Category = "Save")
	void ResetDebug();

	UFUNCTION(BlueprintCallable, Category = "Save")
	void GrantResources(int32 Gold, int32 Wood, int32 SoulAsh, int32 Essence, int32 Embers);

	UFUNCTION(BlueprintCallable, Category = "Buildings")
	int32 GetBuildingLevel(FName BuildingId) const;

	UFUNCTION(BlueprintCallable, Category = "Buildings")
	bool PurchaseBuilding(UBuildingData* BuildingData);

	UFUNCTION(BlueprintCallable, Category = "Buildings")
	float GetPlayerDamageMultiplier() const;

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool BankPendingRunResult();

	UEmberSaveGame* GetCurrentSave() const { return CurrentSave; }
	FString BuildSummary() const;

private:
	UPROPERTY()
	TObjectPtr<UEmberSaveGame> CurrentSave;

	FDelegateHandle GrantResourcesCommandHandle;
	FDelegateHandle ResetSaveCommandHandle;
	FDelegateHandle TravelExpeditionCommandHandle;
	FDelegateHandle TravelCampCommandHandle;
	FDelegateHandle BuyBuildingCommandHandle;

	UEconomySubsystem* GetEconomy() const;
	void LoadOrCreate();
	void SyncEconomyFromSave();
	void SyncSaveFromEconomy();
	void RegisterDebugHooks();
	void UnregisterDebugHooks();
	void GrantResourcesConsole(const TArray<FString>& Args);
	void ResetSaveConsole(const TArray<FString>& Args);
	void TravelExpeditionConsole(const TArray<FString>& Args);
	void TravelCampConsole(const TArray<FString>& Args);
	void BuyBuildingConsole(const TArray<FString>& Args);
	UBuildingData* LoadBuildingData(FName BuildingId) const;
};

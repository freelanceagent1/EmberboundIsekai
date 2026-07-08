#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/MonsterData.h"
#include "MonsterCollectionSubsystem.generated.h"

class UEmberSaveGame;
class UEconomySubsystem;
class USaveManagerSubsystem;

USTRUCT(BlueprintType)
struct EMBERBOUNDISEKAI_API FOfflineRewardResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Offline")
	int32 Minutes = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Offline")
	bool bCapped = false;

	UPROPERTY(BlueprintReadOnly, Category = "Offline")
	TMap<EEmberResource, int32> Rewards;
};

UCLASS()
class EMBERBOUNDISEKAI_API UMonsterCollectionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	const TArray<UMonsterData*>& GetCatalog() const { return Catalog; }
	UMonsterData* FindMonster(FName MonsterId) const;
	int32 GetShardCount(FName MonsterId) const;
	bool IsOwned(FName MonsterId) const;
	EMonsterAssignmentRole GetAssignment(FName MonsterId) const;
	FName GetAssignedMonster(EMonsterAssignmentRole Role) const;
	int32 GetAssignmentSlotCap() const;
	int32 GetUsedAssignmentSlots() const;
	float GetDefenderPower() const;

	UFUNCTION(BlueprintCallable, Category = "Monsters")
	void GrantShards(FName MonsterId, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Monsters")
	bool AssignMonster(FName MonsterId, EMonsterAssignmentRole Role);

	UFUNCTION(BlueprintCallable, Category = "Offline")
	FOfflineRewardResult ApplyOfflineIncome();

	UFUNCTION(BlueprintCallable, Category = "Offline")
	FOfflineRewardResult SimulateOffline(int32 Minutes);

private:
	UPROPERTY()
	TArray<TObjectPtr<UMonsterData>> Catalog;

	FDelegateHandle GrantShardsCommandHandle;
	FDelegateHandle AssignMonsterCommandHandle;
	FDelegateHandle SimulateOfflineCommandHandle;

	void LoadCatalog();
	void CheckUnlock(FName MonsterId);
	void ShowUnlockToast(FName MonsterId);
	void ShowOfflineRewardPopup(const FOfflineRewardResult& Reward);
	void SaveIfAvailable();
	UEmberSaveGame* GetSave() const;
	UEconomySubsystem* GetEconomy() const;
	USaveManagerSubsystem* GetSaveManager() const;
	void RegisterDebugHooks();
	void UnregisterDebugHooks();
	void GrantShardsConsole(const TArray<FString>& Args);
	void AssignMonsterConsole(const TArray<FString>& Args);
	void SimulateOfflineConsole(const TArray<FString>& Args);
	static EMonsterAssignmentRole RoleFromName(const FString& RoleName);
	static FName RoleToName(EMonsterAssignmentRole Role);
	static FString ResourceLogName(EEmberResource Resource);
};

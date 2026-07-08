#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Data/ResourceTypes.h"
#include "DefenseEventManager.generated.h"

class AEmberDefenderActor;
class AEmberEnemyBase;
class UDefenseStatusWidget;
class UEmberEnemyData;
class UHealthComponent;
class UMonsterData;

USTRUCT(BlueprintType)
struct EMBERBOUNDISEKAI_API FDefenseRewardAmounts
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Defense")
	TMap<EEmberResource, int32> Amounts;
};

UCLASS()
class EMBERBOUNDISEKAI_API UDefenseEventManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Defense")
	bool StartDefense();

	UFUNCTION(BlueprintCallable, Category = "Defense")
	void DamageCampfire(float Damage);

	UFUNCTION(BlueprintCallable, Category = "Defense")
	bool IsDefenseActive() const { return bActive; }

	UFUNCTION(BlueprintCallable, Category = "Defense")
	static FDefenseRewardAmounts CalculateRewards(bool bVictory, int32 WatchtowerLevel);

private:
	UPROPERTY()
	TObjectPtr<AActor> CampfireActor;

	UPROPERTY()
	TObjectPtr<UHealthComponent> CampfireHealth;

	UPROPERTY()
	TObjectPtr<UDefenseStatusWidget> StatusWidget;

	UPROPERTY()
	TArray<TObjectPtr<AEmberDefenderActor>> Defenders;

	UPROPERTY()
	TArray<TObjectPtr<AEmberEnemyBase>> SpawnedEnemies;

	FTimerHandle WaveTimerHandle;
	FTimerHandle PollTimerHandle;
	FTimerHandle DebugTimerHandle;
	FDelegateHandle StartDefenseCommandHandle;
	FDelegateHandle DebugAutoDefenseCommandHandle;
	FDelegateHandle DamageCampfireCommandHandle;

	bool bActive = false;
	bool bDebugAutoDefense = false;
	bool bDebugDefeatWave2 = false;
	int32 CurrentWave = 0;
	int32 SpawnedThisWave = 0;
	TArray<FName> PendingWaveEnemyIds;

	void RegisterDebugHooks();
	void UnregisterDebugHooks();
	void StartDefenseConsole(const TArray<FString>& Args);
	void DebugAutoDefenseConsole(const TArray<FString>& Args);
	void DamageCampfireConsole(const TArray<FString>& Args);
	void BeginNextWave();
	void SpawnNextEnemy();
	void PollWaveCompletion();
	void FinishDefense(bool bVictory);
	void SpawnDefenders();
	void CleanupEventActors();
	void EnableCampAutoAttack(bool bEnabled);
	void UpdateStatusWidget();
	void DebugClearWaveEnemies();
	AActor* FindCampfireActor() const;
	UEmberEnemyData* LoadEnemyData(FName EnemyId) const;
	void BuildWave(int32 WaveNumber, TArray<FName>& OutEnemyIds) const;

	UFUNCTION()
	void HandleCampfireDamaged(float Damage, float CurrentHP);

	UFUNCTION()
	void HandleCampfireDied(AActor* DeadActor);
};

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Data/RunUpgradeData.h"
#include "RunUpgradeManager.generated.h"

class AEmberBoneShieldOrbiter;
class AEmberCompanionSlime;
class AEmberPlayerCharacter;
class AEmberSoulFlamePatch;
class UUpgradeChoiceWidget;

USTRUCT(BlueprintType)
struct FEmberProjectileStatusPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BurnDps = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BurnDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PoisonDps = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PoisonDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrostSlowFraction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrostDuration = 0.0f;
};

UCLASS()
class EMBERBOUNDISEKAI_API URunUpgradeManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	TArray<URunUpgradeData*> RollOffer(int32 Count) const;
	void ApplyUpgrade(FName UpgradeId);
	void ApplyUpgrade(URunUpgradeData* Upgrade);
	void NotifyChoiceClosed();
	void ResetRun();

	const FEmberProjectileStatusPayload& GetProjectileStatusPayload() const { return ProjectileStatusPayload; }
	float GetGoldMultiplier() const { return GoldMultiplier; }

private:
	UPROPERTY()
	TArray<TObjectPtr<URunUpgradeData>> UpgradeCatalog;

	UPROPERTY()
	TMap<FName, int32> Stacks;

	UPROPERTY()
	TObjectPtr<UUpgradeChoiceWidget> ActiveChoiceWidget;

	UPROPERTY()
	TArray<TObjectPtr<AEmberBoneShieldOrbiter>> BoneShields;

	UPROPERTY()
	TObjectPtr<AEmberCompanionSlime> CompanionSlime;

	UPROPERTY()
	TArray<TObjectPtr<AEmberSoulFlamePatch>> SoulFlamePatches;

	UPROPERTY()
	TObjectPtr<AEmberPlayerCharacter> Player;

	FEmberProjectileStatusPayload ProjectileStatusPayload;
	int32 PendingLevelUps = 0;
	float GoldMultiplier = 1.0f;
	FTimerHandle PlayerBindTimerHandle;
	FTimerHandle HealingWispTimerHandle;
	FTimerHandle SoulTrailTimerHandle;
	FTimerHandle AutoPickTimerHandle;
	FTimerHandle DebugXPGrantTimerHandle;
	FDelegateHandle GrantXPCommandHandle;

	void LoadCatalog();
	void TryBindPlayer();
	UFUNCTION()
	void HandleLevelUp(int32 NewLevel);
	void ShowNextOffer();
	void AutoPickActiveOffer();
	void RegisterConsoleHooks();
	void UnregisterConsoleHooks();
	void GrantXPConsole(const TArray<FString>& Args);
	void GrantXPToPlayer(int32 Amount);
	void StartDebugXPGrant();
	void HealFromWisp();
	void DropSoulFlamePatch();
	int32 GetStack(FName UpgradeId) const;
	bool CanOffer(const URunUpgradeData* Upgrade) const;
	float GetRarityWeight(ERunUpgradeRarity Rarity) const;
	void SpawnBoneShield();
	void SpawnCompanionSlime();
};

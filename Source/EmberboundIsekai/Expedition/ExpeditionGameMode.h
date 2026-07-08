#pragma once

#include "CoreMinimal.h"
#include "Data/ResourceTypes.h"
#include "GameFramework/GameModeBase.h"
#include "ExpeditionGameMode.generated.h"

class AEmberEnemyBase;
class ABossTutorialDragon;
class AEmberCompanionActor;
class AExpeditionPortal;
class UEmberEnemyData;
class UEmberHUDWidget;
class URunResultWidget;
class UVirtualJoystickWidget;

UENUM()
enum class EEmberRoomType : uint8
{
	Normal,
	Elite,
	Boss
};

USTRUCT()
struct FEmberRoomWave
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FName> EnemyAssetNames;

	UPROPERTY()
	int32 TotalToSpawn = 4;

	UPROPERTY()
	float SpawnInterval = 0.8f;

	UPROPERTY()
	EEmberRoomType RoomType = EEmberRoomType::Normal;
};

UCLASS()
class EMBERBOUNDISEKAI_API AExpeditionGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AExpeditionGameMode();

	virtual void BeginPlay() override;

private:
	static constexpr int32 MaxActiveEnemies = 24;

	UPROPERTY()
	TArray<FEmberRoomWave> Rooms;

	UPROPERTY()
	TObjectPtr<UEmberHUDWidget> HUDWidget;

	UPROPERTY()
	TObjectPtr<UVirtualJoystickWidget> JoystickWidget;

	UPROPERTY()
	TObjectPtr<URunResultWidget> ResultWidget;

	UPROPERTY()
	TObjectPtr<AExpeditionPortal> CurrentPortal;

	UPROPERTY()
	TObjectPtr<ABossTutorialDragon> ActiveBoss;

	UPROPERTY()
	TObjectPtr<AEmberCompanionActor> AssignedCompanion;

	int32 CurrentRoomIndex = 0;
	int32 SpawnedThisRoom = 0;
	int32 KilledThisRoom = 0;
	bool bRunEnded = false;
	bool bResolvedStartRoom = false;
	bool bDebugKillPending = false;
	TMap<EEmberResource, int32> RunResourceTally;
	FDelegateHandle KillPlayerCommandHandle;
	FDelegateHandle DamageBossCommandHandle;
	FTimerHandle AutoDamageBossTimerHandle;
	FTimerHandle DebugKeepAliveTimerHandle;
	FTimerHandle SpawnTimerHandle;
	FTimerHandle RoomCheckTimerHandle;
	TMap<FName, int32> RunMonsterShardTally;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void AddRunResource(EEmberResource Resource, int32 Amount);
	void AddRunMonsterShard(FName MonsterId, int32 Amount);

private:
	void BuildRoomSequence();
	void StartCurrentRoom();
	void SpawnWaveEnemy();
	void SpawnBoss();
	void CheckRoomClear();
	void CompleteRoom();
	void SpawnPortal();
	void EnterNextRoom();
	void GrantRoomReward();
	void EndRun(bool bVictory);
	void BankRunResult(bool bVictory);
	void ShowRunResult();
	void RegisterDebugHooks();
	void UnregisterDebugHooks();
	void KillPlayerConsole(const TArray<FString>& Args);
	void KillPlayerNow();
	void DebugKeepPlayerAlive();
	void DamageBossConsole(const TArray<FString>& Args);
	void AutoDamageBossTick();
	void BindPlayerDeath();
	void SpawnAssignedCompanion();
	void ShowJoystickIfNeeded();
	UFUNCTION()
	void HandlePlayerDied(AActor* DeadActor);
	void HandleBossDefeated(ABossTutorialDragon* Boss);
	UEmberEnemyData* LoadEnemyData(FName AssetName) const;
	FVector PickArenaEdgeSpawnLocation() const;
};

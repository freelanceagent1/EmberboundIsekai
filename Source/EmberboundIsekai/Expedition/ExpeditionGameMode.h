#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ExpeditionGameMode.generated.h"

class AEmberEnemyBase;
class AExpeditionPortal;
class UEmberEnemyData;
class UEmberHUDWidget;

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
	TObjectPtr<AExpeditionPortal> CurrentPortal;

	int32 CurrentRoomIndex = 0;
	int32 SpawnedThisRoom = 0;
	int32 KilledThisRoom = 0;
	FTimerHandle SpawnTimerHandle;
	FTimerHandle RoomCheckTimerHandle;

	void BuildRoomSequence();
	void StartCurrentRoom();
	void SpawnWaveEnemy();
	void CheckRoomClear();
	void CompleteRoom();
	void SpawnPortal();
	void EnterNextRoom();
	void GrantRoomReward();
	UEmberEnemyData* LoadEnemyData(FName AssetName) const;
	FVector PickArenaEdgeSpawnLocation() const;
};

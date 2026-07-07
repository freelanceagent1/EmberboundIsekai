#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EnemySpawnerSubsystem.generated.h"

class AEmberEnemyBase;

UCLASS()
class EMBERBOUNDISEKAI_API UEnemySpawnerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterEnemy(AEmberEnemyBase* Enemy);
	void UnregisterEnemy(AEmberEnemyBase* Enemy);

	const TArray<TObjectPtr<AEmberEnemyBase>>& GetLiveEnemies() const { return LiveEnemies; }
	int32 GetLiveEnemyCount() const { return LiveEnemies.Num(); }

private:
	UPROPERTY()
	TArray<TObjectPtr<AEmberEnemyBase>> LiveEnemies;
};

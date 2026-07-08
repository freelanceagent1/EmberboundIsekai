#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/Poolable.h"
#include "EmberDefenderActor.generated.h"

class UMonsterData;
class UStaticMeshComponent;

UCLASS()
class EMBERBOUNDISEKAI_API AEmberDefenderActor : public AActor, public IPoolable
{
	GENERATED_BODY()

public:
	AEmberDefenderActor();
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnAcquired_Implementation() override;
	virtual void OnReleased_Implementation() override;

	void InitDefender(UMonsterData* InMonsterData);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BillboardVisual;

	UPROPERTY()
	TObjectPtr<UMonsterData> MonsterData;

	FTimerHandle FireTimerHandle;

	void FireAtNearestEnemy();
	void ConfigureVisual();
};

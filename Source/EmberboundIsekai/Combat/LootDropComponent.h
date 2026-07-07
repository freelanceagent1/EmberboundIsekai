#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LootDropComponent.generated.h"

class UEmberEnemyData;

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class EMBERBOUNDISEKAI_API ULootDropComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULootDropComponent();

	void SetEnemyData(UEmberEnemyData* InEnemyData);

	UFUNCTION()
	void HandleOwnerDied(AActor* DeadActor);

private:
	UPROPERTY()
	TObjectPtr<UEmberEnemyData> EnemyData;
};

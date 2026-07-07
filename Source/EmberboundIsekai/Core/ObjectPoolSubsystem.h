#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ObjectPoolSubsystem.generated.h"

USTRUCT()
struct FEmberActorPool
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<AActor>> Actors;
};

UCLASS()
class EMBERBOUNDISEKAI_API UObjectPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Pool")
	AActor* AcquireActor(TSubclassOf<AActor> ActorClass, const FTransform& Transform);

	template <typename T>
	T* Acquire(TSubclassOf<T> ActorClass, const FTransform& Transform)
	{
		return Cast<T>(AcquireActor(ActorClass, Transform));
	}

	UFUNCTION(BlueprintCallable, Category = "Pool")
	void ReleaseActor(AActor* Actor);

private:
	UPROPERTY()
	TMap<TSubclassOf<AActor>, FEmberActorPool> Pool;
};

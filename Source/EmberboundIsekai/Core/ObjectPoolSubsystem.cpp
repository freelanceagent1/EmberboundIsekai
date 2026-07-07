#include "Core/ObjectPoolSubsystem.h"
#include "Core/Poolable.h"

AActor* UObjectPoolSubsystem::AcquireActor(TSubclassOf<AActor> ActorClass, const FTransform& Transform)
{
	if (!GetWorld() || !*ActorClass)
	{
		return nullptr;
	}

	AActor* Actor = nullptr;
	FEmberActorPool& ActorPool = Pool.FindOrAdd(ActorClass);
	while (ActorPool.Actors.Num() > 0 && !Actor)
	{
		Actor = ActorPool.Actors.Pop();
	}

	if (!Actor)
	{
		Actor = GetWorld()->SpawnActor<AActor>(ActorClass, Transform);
	}
	else
	{
		Actor->SetActorTransform(Transform);
		Actor->SetActorHiddenInGame(false);
		Actor->SetActorEnableCollision(true);
		Actor->SetActorTickEnabled(true);
	}

	if (Actor && Actor->GetClass()->ImplementsInterface(UPoolable::StaticClass()))
	{
		IPoolable::Execute_OnAcquired(Actor);
	}
	return Actor;
}

void UObjectPoolSubsystem::ReleaseActor(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}
	if (Actor->GetClass()->ImplementsInterface(UPoolable::StaticClass()))
	{
		IPoolable::Execute_OnReleased(Actor);
	}
	Actor->SetActorHiddenInGame(true);
	Actor->SetActorEnableCollision(false);
	Actor->SetActorTickEnabled(false);
	Pool.FindOrAdd(Actor->GetClass()).Actors.Add(Actor);
}

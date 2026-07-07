#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BossTelegraphActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class EMBERBOUNDISEKAI_API ABossTelegraphActor : public AActor
{
	GENERATED_BODY()

public:
	ABossTelegraphActor();

	void Configure(const FVector& Center, const FRotator& Rotation, float Length, float Width, float Duration);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Visual;
};

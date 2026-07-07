#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EmberSoulFlamePatch.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class EMBERBOUNDISEKAI_API AEmberSoulFlamePatch : public AActor
{
	GENERATED_BODY()

public:
	AEmberSoulFlamePatch();
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Visual;

	float DamageAccumulator = 0.0f;
	TSet<TWeakObjectPtr<AActor>> TouchingEnemies;

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};

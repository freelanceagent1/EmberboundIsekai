#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EmberBoneShieldOrbiter.generated.h"

class AEmberPlayerCharacter;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class EMBERBOUNDISEKAI_API AEmberBoneShieldOrbiter : public AActor
{
	GENERATED_BODY()

public:
	AEmberBoneShieldOrbiter();
	virtual void Tick(float DeltaSeconds) override;
	void AttachToPlayer(AEmberPlayerCharacter* InPlayer, int32 InIndex);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Visual;

	UPROPERTY()
	TObjectPtr<AEmberPlayerCharacter> Player;

	int32 OrbitIndex = 0;
	float OrbitAngle = 0.0f;
	TMap<TWeakObjectPtr<AActor>, float> HitCooldowns;

	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

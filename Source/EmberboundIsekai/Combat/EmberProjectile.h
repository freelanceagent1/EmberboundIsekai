#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/Poolable.h"
#include "Run/RunUpgradeManager.h"
#include "EmberProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class EMBERBOUNDISEKAI_API AEmberProjectile : public AActor, public IPoolable
{
	GENERATED_BODY()

public:
	AEmberProjectile();

	virtual void Tick(float DeltaSeconds) override;
	virtual void OnAcquired_Implementation() override;
	virtual void OnReleased_Implementation() override;

	void Fire(const FVector& Direction, float InDamage, float InSpeed, int32 InPierceCount, int32 InRicochetCount, AActor* InSource, bool bInEnemyProjectile);
	void SetStatusPayload(const FEmberProjectileStatusPayload& InPayload) { StatusPayload = InPayload; }

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Visual;

	UPROPERTY()
	FVector MoveDirection = FVector::ForwardVector;

	UPROPERTY()
	float Damage = 0.0f;

	UPROPERTY()
	float Speed = 1600.0f;

	UPROPERTY()
	int32 PierceRemaining = 0;

	UPROPERTY()
	int32 RicochetRemaining = 0;

	UPROPERTY()
	bool bEnemyProjectile = false;

	UPROPERTY()
	TObjectPtr<AActor> SourceActor;

	UPROPERTY()
	FVector PreviousLocation = FVector::ZeroVector;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> HitActors;

	UPROPERTY()
	FEmberProjectileStatusPayload StatusPayload;

	FTimerHandle LifetimeTimerHandle;

	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void ResolveHit(AActor* OtherActor);
	void TraceMovement(const FVector& From, const FVector& To);
	bool TryRicochet(AActor* HitActor);
	void ReleaseSelf();
};

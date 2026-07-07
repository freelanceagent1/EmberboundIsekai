#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Core/Poolable.h"
#include "Data/EmberEnemyData.h"
#include "EmberEnemyBase.generated.h"

class AEmberPlayerCharacter;
class UFloatingPawnMovement;
class UHealthComponent;
class ULootDropComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class EMBERBOUNDISEKAI_API AEmberEnemyBase : public APawn, public IPoolable
{
	GENERATED_BODY()

public:
	AEmberEnemyBase();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnAcquired_Implementation() override;
	virtual void OnReleased_Implementation() override;

	void InitFromData(UEmberEnemyData* InEnemyData);
	bool IsAlive() const;
	void SetStatusMoveSpeedScale(float InScale);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Visual;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UFloatingPawnMovement> Movement;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ULootDropComponent> LootDropComponent;

	UPROPERTY()
	TObjectPtr<UEmberEnemyData> EnemyData;

	UPROPERTY()
	TObjectPtr<AEmberPlayerCharacter> Player;

	FTimerHandle RepathTimerHandle;
	FTimerHandle RangedTimerHandle;
	TMap<TWeakObjectPtr<AActor>, float> ContactCooldowns;
	FVector DesiredDirection = FVector::ZeroVector;
	float StatusMoveSpeedScale = 1.0f;

	void UpdateDesiredDirection();
	void FireRangedProjectile();

	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleDeath(AActor* DeadActor);
};

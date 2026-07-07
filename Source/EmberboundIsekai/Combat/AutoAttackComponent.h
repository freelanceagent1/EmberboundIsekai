#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AutoAttackComponent.generated.h"

class AEmberEnemyBase;

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class EMBERBOUNDISEKAI_API UAutoAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAutoAttackComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void ApplyAttackSpeedMultiplier(float Multiplier);
	void ApplyDamageMultiplier(float Multiplier);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float Damage = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float AttackInterval = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float Range = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float ProjectileSpeed = 1600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float CritChance = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float CritMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades")
	int32 PierceCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades")
	int32 SplitCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades")
	int32 RicochetCount = 0;

private:
	FTimerHandle AcquisitionTimerHandle;
	FTimerHandle FireTimerHandle;

	UPROPERTY()
	TObjectPtr<AEmberEnemyBase> CurrentTarget;

	void AcquireTarget();
	void FireAtTarget();
	void SpawnProjectile(const FVector& Direction, float ResolvedDamage);
};

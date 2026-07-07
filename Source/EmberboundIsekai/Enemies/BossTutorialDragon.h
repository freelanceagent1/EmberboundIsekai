#pragma once

#include "CoreMinimal.h"
#include "Enemies/EmberEnemyBase.h"
#include "BossTutorialDragon.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnTutorialDragonDefeated, class ABossTutorialDragon*);

UCLASS()
class EMBERBOUNDISEKAI_API ABossTutorialDragon : public AEmberEnemyBase
{
	GENERATED_BODY()

public:
	ABossTutorialDragon();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void DamageBoss(float Amount);
	float GetHealthPercent() const;

	FOnTutorialDragonDefeated OnBossDefeated;

private:
	enum class EBossPattern : uint8
	{
		FireballVolley,
		DashCharge,
		FlameRing
	};

	FTimerHandle PatternTimerHandle;
	FTimerHandle DashTelegraphTimerHandle;
	EBossPattern NextPattern = EBossPattern::FireballVolley;
	FVector DashDirection = FVector::ForwardVector;
	float DashTimeRemaining = 0.0f;
	float DashDamageCooldownUntil = 0.0f;
	bool bEnraged = false;
	bool bDefeatBroadcast = false;

	void CastNextPattern();
	void CastFireballVolley();
	void CastDashCharge();
	void BeginDash();
	void CastFlameRing();
	void FireProjectile(const FVector& Direction, float Damage, float Speed);
	void CheckEnrage();
	void DamagePlayerDuringDash();

	UFUNCTION()
	void HandleBossDamaged(float Damage, float CurrentHP);

	UFUNCTION()
	void HandleBossDied(AActor* DeadActor);
};

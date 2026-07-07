#include "Combat/AutoAttackComponent.h"
#include "Combat/EmberProjectile.h"
#include "Combat/HealthComponent.h"
#include "Core/ObjectPoolSubsystem.h"
#include "Enemies/EmberEnemyBase.h"
#include "Enemies/EnemySpawnerSubsystem.h"
#include "EmberboundIsekai.h"
#include "TimerManager.h"

UAutoAttackComponent::UAutoAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAutoAttackComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(AcquisitionTimerHandle, this, &UAutoAttackComponent::AcquireTarget, 0.15f, true);
		World->GetTimerManager().SetTimer(FireTimerHandle, this, &UAutoAttackComponent::FireAtTarget, AttackInterval, true, 0.25f);
	}
}

void UAutoAttackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AcquisitionTimerHandle);
		World->GetTimerManager().ClearTimer(FireTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void UAutoAttackComponent::AcquireTarget()
{
	CurrentTarget = nullptr;
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return;
	}
	UEnemySpawnerSubsystem* SpawnerSubsystem = World->GetSubsystem<UEnemySpawnerSubsystem>();
	if (!SpawnerSubsystem)
	{
		return;
	}

	float BestDistanceSq = FMath::Square(Range);
	for (AEmberEnemyBase* Enemy : SpawnerSubsystem->GetLiveEnemies())
	{
		if (!Enemy || !Enemy->IsAlive())
		{
			continue;
		}
		const float DistanceSq = FVector::DistSquared2D(Owner->GetActorLocation(), Enemy->GetActorLocation());
		if (DistanceSq <= BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			CurrentTarget = Enemy;
		}
	}
}

void UAutoAttackComponent::FireAtTarget()
{
	AActor* Owner = GetOwner();
	if (!Owner || !CurrentTarget || !CurrentTarget->IsAlive() || !GetWorld())
	{
		return;
	}
	const FVector Direction = (CurrentTarget->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal2D();
	UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
	if (!Pool)
	{
		return;
	}
	const FVector SpawnLocation = Owner->GetActorLocation() + Direction * 70.0f + FVector(0.0f, 0.0f, 45.0f);
	const bool bCrit = FMath::FRand() < CritChance;
	const float ResolvedDamage = bCrit ? Damage * CritMultiplier : Damage;
	if (AEmberProjectile* Projectile = Pool->Acquire<AEmberProjectile>(AEmberProjectile::StaticClass(), FTransform(Direction.Rotation(), SpawnLocation)))
	{
		Projectile->Fire(Direction, ResolvedDamage, ProjectileSpeed, PierceCount, Owner, false);
		if (UHealthComponent* Health = CurrentTarget->FindComponentByClass<UHealthComponent>())
		{
			Health->ApplyDamage(ResolvedDamage);
		}
		UE_LOG(LogEmberbound, Log, TEXT("EMBER: ProjectileFired"));
	}
}

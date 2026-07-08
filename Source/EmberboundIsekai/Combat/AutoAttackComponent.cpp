#include "Combat/AutoAttackComponent.h"
#include "Combat/EmberProjectile.h"
#include "Combat/HealthComponent.h"
#include "Core/ObjectPoolSubsystem.h"
#include "Core/SaveManagerSubsystem.h"
#include "Enemies/EmberEnemyBase.h"
#include "Enemies/EnemySpawnerSubsystem.h"
#include "Run/RunUpgradeManager.h"
#include "EmberboundIsekai.h"
#include "HAL/IConsoleManager.h"
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
	if (USaveManagerSubsystem* Save = GetWorld() && GetWorld()->GetGameInstance() ? GetWorld()->GetGameInstance()->GetSubsystem<USaveManagerSubsystem>() : nullptr)
	{
		ApplyDamageMultiplier(Save->GetPlayerDamageMultiplier());
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

void UAutoAttackComponent::ApplyAttackSpeedMultiplier(float Multiplier)
{
	if (Multiplier <= 0.0f)
	{
		return;
	}
	AttackInterval = FMath::Max(0.12f, AttackInterval * Multiplier);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FireTimerHandle);
		World->GetTimerManager().SetTimer(FireTimerHandle, this, &UAutoAttackComponent::FireAtTarget, AttackInterval, true, AttackInterval);
	}
}

void UAutoAttackComponent::ApplyDamageMultiplier(float Multiplier)
{
	if (Multiplier > 0.0f)
	{
		Damage *= Multiplier;
	}
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
	const bool bCrit = FMath::FRand() < CritChance;
	const float ResolvedDamage = bCrit ? Damage * CritMultiplier : Damage;
	SpawnProjectile(Direction, ResolvedDamage);
	if (SplitCount > 0)
	{
		for (int32 Index = 1; Index <= SplitCount; ++Index)
		{
			const float Angle = 8.0f * static_cast<float>((Index + 1) / 2) * (Index % 2 == 0 ? -1.0f : 1.0f);
			SpawnProjectile(Direction.RotateAngleAxis(Angle, FVector::UpVector), ResolvedDamage * 0.75f);
		}
	}
}

void UAutoAttackComponent::SpawnProjectile(const FVector& Direction, float ResolvedDamage)
{
	AActor* Owner = GetOwner();
	UObjectPoolSubsystem* Pool = GetWorld() ? GetWorld()->GetSubsystem<UObjectPoolSubsystem>() : nullptr;
	if (!Owner || !Pool)
	{
		return;
	}
	const FVector SpawnLocation = Owner->GetActorLocation() + Direction * 70.0f + FVector(0.0f, 0.0f, 45.0f);
	if (AEmberProjectile* Projectile = Pool->Acquire<AEmberProjectile>(AEmberProjectile::StaticClass(), FTransform(Direction.Rotation(), SpawnLocation)))
	{
		float DamageMultiplier = 1.0f;
		if (IConsoleVariable* DamageCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("Ember.PlayerDamageMult")))
		{
			DamageMultiplier = FMath::Max(0.0f, DamageCVar->GetFloat());
		}
		FEmberProjectileStatusPayload StatusPayload;
		if (URunUpgradeManager* UpgradeManager = GetWorld()->GetSubsystem<URunUpgradeManager>())
		{
			StatusPayload = UpgradeManager->GetProjectileStatusPayload();
		}
		Projectile->SetStatusPayload(StatusPayload);
		Projectile->Fire(Direction, ResolvedDamage * DamageMultiplier, ProjectileSpeed, PierceCount, RicochetCount, Owner, false);
		UE_LOG(LogEmberbound, Log, TEXT("EMBER: ProjectileFired"));
	}
}

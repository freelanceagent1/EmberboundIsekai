#include "Combat/AutoAttackComponent.h"
#include "Combat/EmberProjectile.h"
#include "Combat/HealthComponent.h"
#include "Core/ObjectPoolSubsystem.h"
#include "Core/EmberAudioSubsystem.h"
#include "Core/SaveManagerSubsystem.h"
#include "Core/TitleSubsystem.h"
#include "Enemies/EmberEnemyBase.h"
#include "Enemies/EnemySpawnerSubsystem.h"
#include "Run/RunUpgradeManager.h"
#include "EmberboundIsekai.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "TimerManager.h"

UAutoAttackComponent::UAutoAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAutoAttackComponent::BeginPlay()
{
	Super::BeginPlay();
	RefreshTimers();
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
	RefreshTimers();
}

void UAutoAttackComponent::SetAttackingEnabled(bool bEnabled)
{
	bAttackingEnabled = bEnabled;
	if (!bAttackingEnabled)
	{
		CurrentTarget = nullptr;
	}
	RefreshTimers();
}

void UAutoAttackComponent::RefreshTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AcquisitionTimerHandle);
		World->GetTimerManager().ClearTimer(FireTimerHandle);
		if (bAttackingEnabled)
		{
			World->GetTimerManager().SetTimer(AcquisitionTimerHandle, this, &UAutoAttackComponent::AcquireTarget, 0.15f, true);
			World->GetTimerManager().SetTimer(FireTimerHandle, this, &UAutoAttackComponent::FireAtTarget, AttackInterval, true, 0.25f);
		}
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
		float TitleDamage = ResolvedDamage;
		if (UTitleSubsystem* Titles = GetWorld()->GetGameInstance() ? GetWorld()->GetGameInstance()->GetSubsystem<UTitleSubsystem>() : nullptr)
		{
			float HealthFraction = 1.0f;
			if (UHealthComponent* OwnerHealth = Owner->FindComponentByClass<UHealthComponent>())
			{
				HealthFraction = OwnerHealth->GetMaxHP() > 0.0f ? OwnerHealth->GetCurrentHP() / OwnerHealth->GetMaxHP() : 1.0f;
			}
			TitleDamage = Titles->ApplyPlayerDamage(ResolvedDamage, HealthFraction);
			if (!FMath::IsNearlyEqual(TitleDamage, ResolvedDamage))
			{
				UE_LOG(LogEmberbound, Log, TEXT("EMBER: PlayerDamageAdjusted base=%.2f adjusted=%.2f hpfrac=%.2f"), ResolvedDamage, TitleDamage, HealthFraction);
			}
		}
		FEmberProjectileStatusPayload StatusPayload;
		if (URunUpgradeManager* UpgradeManager = GetWorld()->GetSubsystem<URunUpgradeManager>())
		{
			StatusPayload = UpgradeManager->GetProjectileStatusPayload();
		}
		Projectile->SetStatusPayload(StatusPayload);
		Projectile->Fire(Direction, TitleDamage * DamageMultiplier, ProjectileSpeed, PierceCount, RicochetCount, Owner, false);
		if (UEmberAudioSubsystem* Audio = GetWorld()->GetGameInstance() ? GetWorld()->GetGameInstance()->GetSubsystem<UEmberAudioSubsystem>() : nullptr)
		{
			Audio->PlaySfx(TEXT("arrow-shot"), Owner);
		}
		UE_LOG(LogEmberbound, Log, TEXT("EMBER: ProjectileFired"));
	}
}

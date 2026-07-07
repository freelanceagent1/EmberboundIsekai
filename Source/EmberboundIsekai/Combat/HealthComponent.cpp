#include "Combat/HealthComponent.h"
#include "Enemies/EmberEnemyBase.h"
#include "Components/MeshComponent.h"
#include "TimerManager.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHP = MaxHP;
	CacheDynamicMaterials();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(StatusTickTimerHandle, this, &UHealthComponent::TickStatusEffects, 0.25f, true);
	}
}

void UHealthComponent::SetMaxHealth(float NewMaxHealth, bool bHealToFull)
{
	MaxHP = FMath::Max(1.0f, NewMaxHealth);
	if (bHealToFull)
	{
		CurrentHP = MaxHP;
	}
	else
	{
		CurrentHP = FMath::Clamp(CurrentHP, 0.0f, MaxHP);
	}
}

void UHealthComponent::ApplyDamage(float Damage)
{
	if (Damage <= 0.0f || !IsAlive())
	{
		return;
	}
	CurrentHP = FMath::Max(0.0f, CurrentHP - Damage);
	SetFlash(1.0f);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(FlashTimerHandle, FTimerDelegate::CreateUObject(this, &UHealthComponent::SetFlash, 0.0f), 0.08f, false);
	}
	OnDamaged.Broadcast(Damage, CurrentHP);
	if (CurrentHP <= 0.0f)
	{
		OnDied.Broadcast(GetOwner());
	}
}

void UHealthComponent::Heal(float Amount)
{
	if (Amount > 0.0f && IsAlive())
	{
		CurrentHP = FMath::Min(MaxHP, CurrentHP + Amount);
	}
}

void UHealthComponent::AddDamageStatus(EEmberStatusEffectType Type, float Dps, float Duration)
{
	if (Dps <= 0.0f || Duration <= 0.0f || !IsAlive())
	{
		return;
	}
	FEmberActiveStatusEffect& Added = ActiveStatusEffects.AddDefaulted_GetRef();
	Added.Type = Type;
	Added.Dps = Dps;
	Added.RemainingDuration = Duration;
}

void UHealthComponent::AddFrostStatus(float SlowFraction, float Duration)
{
	if (SlowFraction <= 0.0f || Duration <= 0.0f || !IsAlive())
	{
		return;
	}
	for (FEmberActiveStatusEffect& Effect : ActiveStatusEffects)
	{
		if (Effect.Type == EEmberStatusEffectType::Frost)
		{
			Effect.SlowFraction = FMath::Max(Effect.SlowFraction, SlowFraction);
			Effect.RemainingDuration = FMath::Max(Effect.RemainingDuration, Duration);
			UpdateOwnerSlow();
			return;
		}
	}
	FEmberActiveStatusEffect& Added = ActiveStatusEffects.AddDefaulted_GetRef();
	Added.Type = EEmberStatusEffectType::Frost;
	Added.SlowFraction = SlowFraction;
	Added.RemainingDuration = Duration;
	UpdateOwnerSlow();
}

void UHealthComponent::CacheDynamicMaterials()
{
	DynamicMaterials.Reset();
	if (!GetOwner())
	{
		return;
	}
	TArray<UMeshComponent*> Meshes;
	GetOwner()->GetComponents<UMeshComponent>(Meshes);
	for (UMeshComponent* Mesh : Meshes)
	{
		for (int32 Index = 0; Index < Mesh->GetNumMaterials(); ++Index)
		{
			if (UMaterialInterface* Material = Mesh->GetMaterial(Index))
			{
				DynamicMaterials.Add(Mesh->CreateDynamicMaterialInstance(Index, Material));
			}
		}
	}
}

void UHealthComponent::SetFlash(float Value)
{
	for (UMaterialInstanceDynamic* Material : DynamicMaterials)
	{
		if (Material)
		{
			Material->SetScalarParameterValue(TEXT("Flash"), Value);
		}
	}
}

void UHealthComponent::TickStatusEffects()
{
	if (!IsAlive())
	{
		ActiveStatusEffects.Reset();
		UpdateOwnerSlow();
		return;
	}
	float DamageThisTick = 0.0f;
	for (int32 Index = ActiveStatusEffects.Num() - 1; Index >= 0; --Index)
	{
		FEmberActiveStatusEffect& Effect = ActiveStatusEffects[Index];
		Effect.RemainingDuration -= 0.25f;
		if (Effect.Type == EEmberStatusEffectType::Burn || Effect.Type == EEmberStatusEffectType::Poison)
		{
			DamageThisTick += Effect.Dps * 0.25f;
		}
		if (Effect.RemainingDuration <= 0.0f)
		{
			ActiveStatusEffects.RemoveAtSwap(Index);
		}
	}
	if (DamageThisTick > 0.0f)
	{
		ApplyDamage(DamageThisTick);
	}
	UpdateOwnerSlow();
}

void UHealthComponent::UpdateOwnerSlow()
{
	float StrongestSlow = 0.0f;
	for (const FEmberActiveStatusEffect& Effect : ActiveStatusEffects)
	{
		if (Effect.Type == EEmberStatusEffectType::Frost)
		{
			StrongestSlow = FMath::Max(StrongestSlow, Effect.SlowFraction);
		}
	}
	if (AEmberEnemyBase* Enemy = Cast<AEmberEnemyBase>(GetOwner()))
	{
		Enemy->SetStatusMoveSpeedScale(1.0f - StrongestSlow);
	}
}

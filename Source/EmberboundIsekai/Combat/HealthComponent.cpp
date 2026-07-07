#include "Combat/HealthComponent.h"
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

#include "Combat/LootDropComponent.h"
#include "Combat/EmberPickup.h"
#include "Core/ObjectPoolSubsystem.h"
#include "Data/EmberEnemyData.h"

ULootDropComponent::ULootDropComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULootDropComponent::SetEnemyData(UEmberEnemyData* InEnemyData)
{
	EnemyData = InEnemyData;
}

void ULootDropComponent::HandleOwnerDied(AActor* DeadActor)
{
	if (!EnemyData || !DeadActor || !GetWorld())
	{
		return;
	}
	UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
	if (!Pool)
	{
		return;
	}

	const FVector BaseLocation = DeadActor->GetActorLocation();
	if (AEmberPickup* XP = Pool->Acquire<AEmberPickup>(AEmberPickup::StaticClass(), FTransform(BaseLocation + FVector(0.0f, 0.0f, 35.0f))))
	{
		XP->Configure(EEmberPickupType::XP, EnemyData->XPReward);
	}

	for (const FEnemyResourceDrop& Drop : EnemyData->ResourceDrops)
	{
		if (FMath::FRand() <= Drop.Probability)
		{
			const int32 Amount = FMath::RandRange(Drop.MinAmount, FMath::Max(Drop.MinAmount, Drop.MaxAmount));
			EEmberPickupType Type = EEmberPickupType::Gold;
			if (Drop.Resource == EEmberResource::Wood)
			{
				Type = EEmberPickupType::Wood;
			}
			else if (Drop.Resource == EEmberResource::SoulAsh)
			{
				Type = EEmberPickupType::SoulAsh;
			}
			if (AEmberPickup* Pickup = Pool->Acquire<AEmberPickup>(AEmberPickup::StaticClass(), FTransform(BaseLocation + FMath::VRand() * 60.0f)))
			{
				Pickup->Configure(Type, Amount);
			}
		}
	}
}

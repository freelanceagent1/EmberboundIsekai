#include "Combat/XPComponent.h"
#include "Combat/HealthComponent.h"
#include "EmberboundIsekai.h"

UXPComponent::UXPComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UXPComponent::AddXP(int32 Amount)
{
	XP += FMath::Max(0, Amount);
	while (XP >= GetXPToNext())
	{
		XP -= GetXPToNext();
		++Level;
		if (UHealthComponent* Health = GetOwner()->FindComponentByClass<UHealthComponent>())
		{
			Health->Heal(10.0f);
		}
		UE_LOG(LogEmberbound, Log, TEXT("EMBER: LevelUp Level=%d"), Level);
		OnLevelUp.Broadcast(Level);
	}
}

int32 UXPComponent::GetXPToNext() const
{
	return FMath::Max(1, FMath::RoundToInt(10.0f * FMath::Pow(1.35f, static_cast<float>(Level - 1))));
}

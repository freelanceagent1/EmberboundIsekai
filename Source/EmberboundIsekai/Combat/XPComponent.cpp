#include "Combat/XPComponent.h"
#include "Combat/HealthComponent.h"
#include "Core/EmberAudioSubsystem.h"
#include "Core/TitleSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "EmberboundIsekai.h"

UXPComponent::UXPComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UXPComponent::AddXP(int32 Amount)
{
	int32 AdjustedAmount = FMath::Max(0, Amount);
	if (UWorld* World = GetWorld())
	{
		if (UTitleSubsystem* Titles = World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<UTitleSubsystem>() : nullptr)
		{
			AdjustedAmount = Titles->ApplyXpGain(AdjustedAmount);
			if (AdjustedAmount != Amount)
			{
				UE_LOG(LogEmberbound, Log, TEXT("EMBER: XpAdjusted base=%d adjusted=%d"), Amount, AdjustedAmount);
			}
		}
	}
	XP += AdjustedAmount;
	while (XP >= GetXPToNext())
	{
		XP -= GetXPToNext();
		++Level;
		if (UHealthComponent* Health = GetOwner()->FindComponentByClass<UHealthComponent>())
		{
			Health->Heal(10.0f);
		}
		if (UWorld* AudioWorld = GetWorld())
		{
			if (UEmberAudioSubsystem* Audio = AudioWorld->GetGameInstance() ? AudioWorld->GetGameInstance()->GetSubsystem<UEmberAudioSubsystem>() : nullptr)
			{
				Audio->PlaySfx(TEXT("level-up"), GetOwner());
			}
		}
		UE_LOG(LogEmberbound, Log, TEXT("EMBER: LevelUp Level=%d"), Level);
		OnLevelUp.Broadcast(Level);
	}
}

int32 UXPComponent::GetXPToNext() const
{
	return FMath::Max(1, FMath::RoundToInt(10.0f * FMath::Pow(1.35f, static_cast<float>(Level - 1))));
}

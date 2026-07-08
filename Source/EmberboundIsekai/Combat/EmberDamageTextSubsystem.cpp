#include "Combat/EmberDamageTextSubsystem.h"
#include "Combat/EmberDamageTextActor.h"
#include "Core/ObjectPoolSubsystem.h"
#include "Engine/World.h"
#include "EmberboundIsekai.h"

void UEmberDamageTextSubsystem::Tick(float DeltaTime)
{
	for (int32 Index = ActiveTexts.Num() - 1; Index >= 0; --Index)
	{
		FActiveDamageText& Entry = ActiveTexts[Index];
		Entry.RemainingSeconds -= DeltaTime;
		if (Entry.RemainingSeconds <= 0.0f || !Entry.Actor.IsValid())
		{
			ReleaseAt(Index);
		}
	}
	LogCooldownSeconds = FMath::Max(0.0f, LogCooldownSeconds - DeltaTime);
}

TStatId UEmberDamageTextSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UEmberDamageTextSubsystem, STATGROUP_Tickables);
}

void UEmberDamageTextSubsystem::ShowDamage(AActor* DamagedActor, float Damage)
{
	if (!DamagedActor || Damage <= 0.0f || !GetWorld())
	{
		return;
	}
	UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
	if (!Pool)
	{
		return;
	}
	while (ActiveTexts.Num() >= MaxConcurrentTexts)
	{
		ReleaseAt(0);
	}

	const FVector SpawnLocation = DamagedActor->GetActorLocation() + FVector(0.0f, 0.0f, 110.0f);
	AEmberDamageTextActor* TextActor = Pool->Acquire<AEmberDamageTextActor>(AEmberDamageTextActor::StaticClass(), FTransform(FRotator::ZeroRotator, SpawnLocation));
	if (!TextActor)
	{
		return;
	}
	TextActor->ShowDamage(Damage, IsCriticalDamage(Damage));

	FActiveDamageText& Entry = ActiveTexts.AddDefaulted_GetRef();
	Entry.Actor = TextActor;
	Entry.RemainingSeconds = 0.6f;

	if (LogCooldownSeconds <= 0.0f)
	{
		UE_LOG(LogEmberbound, Log, TEXT("EMBER: DamageText n=%d"), ActiveTexts.Num());
		LogCooldownSeconds = 0.5f;
	}
}

void UEmberDamageTextSubsystem::ReleaseAt(int32 Index)
{
	if (!ActiveTexts.IsValidIndex(Index))
	{
		return;
	}
	if (AEmberDamageTextActor* Actor = ActiveTexts[Index].Actor.Get())
	{
		if (UObjectPoolSubsystem* Pool = GetWorld() ? GetWorld()->GetSubsystem<UObjectPoolSubsystem>() : nullptr)
		{
			Pool->ReleaseActor(Actor);
		}
	}
	ActiveTexts.RemoveAtSwap(Index, 1, EAllowShrinking::No);
}

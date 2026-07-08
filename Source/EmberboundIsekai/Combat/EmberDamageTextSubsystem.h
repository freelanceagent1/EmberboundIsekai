#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EmberDamageTextSubsystem.generated.h"

class AEmberDamageTextActor;

UCLASS()
class EMBERBOUNDISEKAI_API UEmberDamageTextSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	static constexpr int32 MaxConcurrentTexts = 32;
	static constexpr float CriticalDamageThreshold = 25.0f;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	static int32 GetMaxConcurrentTexts() { return MaxConcurrentTexts; }
	static bool IsCriticalDamage(float Damage) { return Damage >= CriticalDamageThreshold; }

	void ShowDamage(AActor* DamagedActor, float Damage);

private:
	struct FActiveDamageText
	{
		TWeakObjectPtr<AEmberDamageTextActor> Actor;
		float RemainingSeconds = 0.6f;
	};

	TArray<FActiveDamageText> ActiveTexts;
	float LogCooldownSeconds = 0.0f;

	void ReleaseAt(int32 Index);
};

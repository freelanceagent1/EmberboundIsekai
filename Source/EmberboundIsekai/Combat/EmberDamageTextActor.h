#pragma once

#include "CoreMinimal.h"
#include "Core/Poolable.h"
#include "GameFramework/Actor.h"
#include "EmberDamageTextActor.generated.h"

class UTextRenderComponent;

UCLASS()
class EMBERBOUNDISEKAI_API AEmberDamageTextActor : public AActor, public IPoolable
{
	GENERATED_BODY()

public:
	AEmberDamageTextActor();

	virtual void Tick(float DeltaSeconds) override;
	virtual void OnAcquired_Implementation() override;
	virtual void OnReleased_Implementation() override;

	void ShowDamage(float Damage, bool bCritical);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTextRenderComponent> TextComponent;

	float Lifetime = 0.0f;
	float MaxLifetime = 0.6f;
	FVector BaseLocation = FVector::ZeroVector;
	FLinearColor BaseColor = FLinearColor::White;
};

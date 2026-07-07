#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/Poolable.h"
#include "Data/ResourceTypes.h"
#include "EmberPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class AEmberPlayerCharacter;

UENUM(BlueprintType)
enum class EEmberPickupType : uint8
{
	XP,
	Gold,
	Wood,
	SoulAsh
};

UCLASS()
class EMBERBOUNDISEKAI_API AEmberPickup : public AActor, public IPoolable
{
	GENERATED_BODY()

public:
	AEmberPickup();

	virtual void Tick(float DeltaSeconds) override;
	virtual void OnReleased_Implementation() override;

	void Configure(EEmberPickupType InType, int32 InAmount);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Visual;

	UPROPERTY()
	EEmberPickupType PickupType = EEmberPickupType::XP;

	UPROPERTY()
	int32 Amount = 1;

	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void Collect(AEmberPlayerCharacter* Player);
};

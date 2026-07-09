#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExpeditionPortal.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class EMBERBOUNDISEKAI_API AExpeditionPortal : public AActor
{
	GENERATED_BODY()

public:
	AExpeditionPortal();
	virtual void Tick(float DeltaSeconds) override;

	DECLARE_DELEGATE(FOnPortalEntered);
	FOnPortalEntered OnPortalEntered;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Visual;

	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> PortalMID;

	bool bUsed = false;

	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

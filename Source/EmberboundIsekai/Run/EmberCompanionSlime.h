#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "EmberCompanionSlime.generated.h"

class AEmberPlayerCharacter;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class EMBERBOUNDISEKAI_API AEmberCompanionSlime : public APawn
{
	GENERATED_BODY()

public:
	AEmberCompanionSlime();
	virtual void Tick(float DeltaSeconds) override;
	void SetPlayer(AEmberPlayerCharacter* InPlayer) { Player = InPlayer; }

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Visual;

	UPROPERTY()
	TObjectPtr<AEmberPlayerCharacter> Player;

	FTimerHandle FireTimerHandle;
	void FireAcidBlob();
};

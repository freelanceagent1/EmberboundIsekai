#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "EmberCompanionActor.generated.h"

class AEmberPlayerCharacter;
class UMonsterData;
class USphereComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

UCLASS()
class EMBERBOUNDISEKAI_API AEmberCompanionActor : public APawn
{
	GENERATED_BODY()

public:
	AEmberCompanionActor();
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitCompanion(UMonsterData* InMonsterData, AEmberPlayerCharacter* InPlayer);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Visual;

	UPROPERTY()
	TObjectPtr<UMonsterData> MonsterData;

	UPROPERTY()
	TObjectPtr<AEmberPlayerCharacter> Player;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> VisualMID;

	FTimerHandle FireTimerHandle;

	void FireProjectile();
	void ConfigureVisual();
};

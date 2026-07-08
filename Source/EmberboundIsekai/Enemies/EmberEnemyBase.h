#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Core/Poolable.h"
#include "Data/EmberEnemyData.h"
#include "EmberEnemyBase.generated.h"

class AEmberPlayerCharacter;
class UFloatingPawnMovement;
class UHealthComponent;
class ULootDropComponent;
class UMaterialInstanceDynamic;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class EMBERBOUNDISEKAI_API AEmberEnemyBase : public APawn, public IPoolable
{
	GENERATED_BODY()

public:
	AEmberEnemyBase();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnAcquired_Implementation() override;
	virtual void OnReleased_Implementation() override;

	void InitFromData(UEmberEnemyData* InEnemyData);
	void ApplyRuntimeModifiers(float HealthMultiplier, float DamageMultiplier, float ScaleMultiplier, const FLinearColor& TintOverride);
	void ConfigureRuntimeStats(float MaxHealth, float ContactDamage, float MoveSpeed, const FVector& VisualScale, const FLinearColor& Tint);
	bool IsAlive() const;
	void SetStatusMoveSpeedScale(float InScale);
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Visual;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BillboardVisual;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BlobShadow;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UFloatingPawnMovement> Movement;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ULootDropComponent> LootDropComponent;

	UPROPERTY()
	TObjectPtr<UEmberEnemyData> EnemyData;

	UPROPERTY()
	TObjectPtr<AEmberPlayerCharacter> Player;

	FTimerHandle RepathTimerHandle;
	FTimerHandle RangedTimerHandle;
	TMap<TWeakObjectPtr<AActor>, float> ContactCooldowns;
	FVector DesiredDirection = FVector::ZeroVector;
	float StatusMoveSpeedScale = 1.0f;
	float RuntimeContactDamage = 0.0f;
	float RuntimeMoveSpeed = 250.0f;

	void UpdateDesiredDirection();
	void FireRangedProjectile();
	void ConfigureBillboard(FName TextureId, const FVector& VisualScale, const FLinearColor& Tint);
	void ResetBillboardFlash();

	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleDamaged(float Damage, float CurrentHP);

	UFUNCTION()
	void HandleDeath(AActor* DeadActor);

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BillboardMID;

	FTimerHandle BillboardFlashTimerHandle;
};

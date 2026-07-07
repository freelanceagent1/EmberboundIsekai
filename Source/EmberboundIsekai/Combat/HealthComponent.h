#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

UENUM(BlueprintType)
enum class EEmberStatusEffectType : uint8
{
	Burn,
	Poison,
	Frost
};

USTRUCT(BlueprintType)
struct FEmberActiveStatusEffect
{
	GENERATED_BODY()

	UPROPERTY()
	EEmberStatusEffectType Type = EEmberStatusEffectType::Burn;

	UPROPERTY()
	float Dps = 0.0f;

	UPROPERTY()
	float RemainingDuration = 0.0f;

	UPROPERTY()
	float SlowFraction = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthDied, AActor*, DeadActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthDamaged, float, Damage, float, CurrentHP);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class EMBERBOUNDISEKAI_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetMaxHealth(float NewMaxHealth, bool bHealToFull);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ApplyDamage(float Damage);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float Amount);

	void AddDamageStatus(EEmberStatusEffectType Type, float Dps, float Duration);
	void AddFrostStatus(float SlowFraction, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Health")
	bool IsAlive() const { return CurrentHP > 0.0f; }

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetCurrentHP() const { return CurrentHP; }

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetMaxHP() const { return MaxHP; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHP = 100.0f;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthDied OnDied;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthDamaged OnDamaged;

private:
	UPROPERTY()
	float CurrentHP = 100.0f;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> DynamicMaterials;

	UPROPERTY()
	TArray<FEmberActiveStatusEffect> ActiveStatusEffects;

	FTimerHandle FlashTimerHandle;
	FTimerHandle StatusTickTimerHandle;

	void CacheDynamicMaterials();
	void SetFlash(float Value);
	void TickStatusEffects();
	void UpdateOwnerSlow();
};

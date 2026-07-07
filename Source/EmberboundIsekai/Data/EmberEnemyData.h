#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ResourceTypes.h"
#include "EmberEnemyData.generated.h"

class AEmberEnemyBase;

UENUM(BlueprintType)
enum class EEmberEnemyAttackType : uint8
{
	Melee,
	Ranged
};

USTRUCT(BlueprintType)
struct EMBERBOUNDISEKAI_API FEnemyResourceDrop
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop")
	EEmberResource Resource = EEmberResource::Gold;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Probability = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "0"))
	int32 MinAmount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "0"))
	int32 MaxAmount = 1;
};

UCLASS(BlueprintType)
class EMBERBOUNDISEKAI_API UEmberEnemyData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy", meta = (ClampMin = "1.0"))
	float MaxHP = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy", meta = (ClampMin = "0.0"))
	float ContactDamage = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy", meta = (ClampMin = "0.0"))
	float MoveSpeed = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	EEmberEnemyAttackType AttackType = EEmberEnemyAttackType::Melee;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float ProjectileDamage = 9.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float ProjectileSpeed = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float ProjectileInterval = 1.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	int32 XPReward = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	TArray<FEnemyResourceDrop> ResourceDrops;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	FLinearColor ColorTint = FLinearColor::Green;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	FVector VisualScale = FVector(1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	TSubclassOf<AEmberEnemyBase> EnemyClassOverride;
};

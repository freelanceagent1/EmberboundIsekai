#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RunUpgradeData.generated.h"

UENUM(BlueprintType)
enum class ERunUpgradeRarity : uint8
{
	Common,
	Rare,
	Epic
};

UENUM(BlueprintType)
enum class ERunUpgradeEffectType : uint8
{
	SplitShot,
	PiercingShot,
	RicochetShot,
	RapidFire,
	HeavyArrow,
	FireArrow,
	PoisonArrow,
	FrostArrow,
	BoneShield,
	HealingWisp,
	PickupMagnet,
	CritFocus,
	CompanionSlime,
	SoulFlameTrail,
	GoldRush
};

UCLASS(BlueprintType)
class EMBERBOUNDISEKAI_API URunUpgradeData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	ERunUpgradeRarity Rarity = ERunUpgradeRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	int32 MaxStacks = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	ERunUpgradeEffectType EffectType = ERunUpgradeEffectType::RapidFire;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	TMap<FName, float> FloatParams;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	TSoftObjectPtr<UTexture2D> Icon;
};

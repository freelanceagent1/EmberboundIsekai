#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TitleData.generated.h"

UENUM(BlueprintType)
enum class ETitlePassiveEffectType : uint8
{
	None,
	OfflineProductionPercent,
	RevivePerRun,
	LowHealthDamagePercent,
	UpgradeCostPercent,
	XpGainPercent
};

UENUM(BlueprintType)
enum class ETitleUnlockCondition : uint8
{
	None,
	OfflineRewardCollected,
	BossDefeated,
	BuildingUpgrades,
	RunUpgradesPicked
};

USTRUCT(BlueprintType)
struct EMBERBOUNDISEKAI_API FTitlePassiveSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title")
	ETitlePassiveEffectType EffectType = ETitlePassiveEffectType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title")
	float Magnitude = 0.0f;
};

UCLASS(BlueprintType)
class EMBERBOUNDISEKAI_API UTitleData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("EmberTitle"), Id);
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title")
	FTitlePassiveSpec Passive;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title")
	ETitleUnlockCondition UnlockCondition = ETitleUnlockCondition::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title")
	int32 UnlockParam = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Title")
	bool bDebugOnly = false;
};

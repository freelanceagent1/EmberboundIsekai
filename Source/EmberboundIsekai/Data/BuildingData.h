#pragma once

#include "CoreMinimal.h"
#include "Data/ResourceTypes.h"
#include "Engine/DataAsset.h"
#include "BuildingData.generated.h"

UCLASS(BlueprintType)
class EMBERBOUNDISEKAI_API UBuildingData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	int32 MaxLevel = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	TArray<FResourceCost> UpgradeCosts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	TArray<float> PerLevelEffectMagnitude;
};

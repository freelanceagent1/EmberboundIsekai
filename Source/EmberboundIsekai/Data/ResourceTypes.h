#pragma once

#include "CoreMinimal.h"
#include "ResourceTypes.generated.h"

UENUM(BlueprintType)
enum class EEmberResource : uint8
{
	Gold,
	Wood,
	SoulAsh,
	MonsterEssence,
	Embers
};

USTRUCT(BlueprintType)
struct EMBERBOUNDISEKAI_API FResourceCost
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
	TMap<EEmberResource, int32> Amounts;
};

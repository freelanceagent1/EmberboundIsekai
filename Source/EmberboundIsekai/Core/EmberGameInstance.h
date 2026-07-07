#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Data/ResourceTypes.h"
#include "EmberGameInstance.generated.h"

USTRUCT(BlueprintType)
struct EMBERBOUNDISEKAI_API FEmberRunResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Run")
	bool bRunComplete = false;

	UPROPERTY(BlueprintReadWrite, Category = "Run")
	int32 RoomsCleared = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Run")
	TMap<EEmberResource, int32> EarnedResources;
};

UCLASS()
class EMBERBOUNDISEKAI_API UEmberGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UPROPERTY(BlueprintReadWrite, Category = "Run")
	FEmberRunResult LastRunResult;
};

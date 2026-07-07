#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Poolable.generated.h"

UINTERFACE(BlueprintType)
class UPoolable : public UInterface
{
	GENERATED_BODY()
};

class EMBERBOUNDISEKAI_API IPoolable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pool")
	void OnAcquired();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pool")
	void OnReleased();
};

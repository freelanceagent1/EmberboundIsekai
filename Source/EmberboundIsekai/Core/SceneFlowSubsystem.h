#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SceneFlowSubsystem.generated.h"

UCLASS()
class EMBERBOUNDISEKAI_API USceneFlowSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Scene Flow")
	void TravelToExpedition();

	UFUNCTION(BlueprintCallable, Category = "Scene Flow")
	void TravelToCamp();
};

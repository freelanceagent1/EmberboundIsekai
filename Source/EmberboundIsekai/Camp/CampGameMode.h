#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CampGameMode.generated.h"

class UCampHUDWidget;

UCLASS()
class EMBERBOUNDISEKAI_API ACampGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACampGameMode();
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<UCampHUDWidget> CampHUD;

	void SpawnCampActors();
};

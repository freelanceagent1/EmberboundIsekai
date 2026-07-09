#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CampGameMode.generated.h"

class UCampHUDWidget;
class UVirtualJoystickWidget;

UCLASS()
class EMBERBOUNDISEKAI_API ACampGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACampGameMode();
	virtual void BeginPlay() override;
	void StartCampAfterMenu();

private:
	UPROPERTY()
	TObjectPtr<UCampHUDWidget> CampHUD;

	UPROPERTY()
	TObjectPtr<UVirtualJoystickWidget> JoystickWidget;

	bool bCampPresentationStarted = false;

	void SpawnCampActors();
	void ShowJoystickIfNeeded();
};

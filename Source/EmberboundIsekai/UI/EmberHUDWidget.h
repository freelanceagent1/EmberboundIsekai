#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ResourceTypes.h"
#include "EmberHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class EMBERBOUNDISEKAI_API UEmberHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void SetRoom(int32 Room, int32 MaxRooms);

private:
	UPROPERTY()
	TObjectPtr<UProgressBar> HPBar;

	UPROPERTY()
	TObjectPtr<UProgressBar> XPBar;

	UPROPERTY()
	TObjectPtr<UTextBlock> LevelText;

	UPROPERTY()
	TObjectPtr<UTextBlock> RoomText;

	UPROPERTY()
	TObjectPtr<UTextBlock> ResourceText;

	int32 CurrentRoom = 1;
	int32 TotalRooms = 5;
};

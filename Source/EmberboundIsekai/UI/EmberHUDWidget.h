#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ResourceTypes.h"
#include "EmberHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UHealthComponent;

UCLASS()
class EMBERBOUNDISEKAI_API UEmberHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void SetRoom(int32 Room, int32 MaxRooms);
	void SetBossHealthSource(UHealthComponent* InBossHealth);
	void ClearBossHealthSource();

private:
	UPROPERTY()
	TObjectPtr<UProgressBar> HPBar;

	UPROPERTY()
	TObjectPtr<UProgressBar> XPBar;

	UPROPERTY()
	TObjectPtr<UProgressBar> BossHPBar;

	UPROPERTY()
	TObjectPtr<UTextBlock> BossText;

	UPROPERTY()
	TObjectPtr<UTextBlock> LevelText;

	UPROPERTY()
	TObjectPtr<UTextBlock> RoomText;

	UPROPERTY()
	TObjectPtr<UTextBlock> ResourceText;

	int32 CurrentRoom = 1;
	int32 TotalRooms = 5;

	UPROPERTY()
	TWeakObjectPtr<UHealthComponent> BossHealth;
};

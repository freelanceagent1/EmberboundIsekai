#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ResourceTypes.h"
#include "CampHUDWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class EMBERBOUNDISEKAI_API UCampHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

private:
	UPROPERTY()
	TMap<EEmberResource, TObjectPtr<UTextBlock>> ResourceTexts;

	UPROPERTY()
	TObjectPtr<UTextBlock> StubText;

	void RefreshResources();
	UButton* MakeButton(const FText& Label, FName Name);

	UFUNCTION()
	void HandleExpedition();
	UFUNCTION()
	void HandleUpgrades();
	UFUNCTION()
	void HandleMonsters();
	UFUNCTION()
	void HandleTitles();
	UFUNCTION()
	void HandleDebug();
	UFUNCTION()
	void HandleDefense();
	UFUNCTION()
	void HandleSave();
	UFUNCTION()
	void HandleReset();
	UFUNCTION()
	void HandleDebugGrantResources();
	UFUNCTION()
	void HandleDebugGrantShards();
	UFUNCTION()
	void HandleDebugOffline();
	UFUNCTION()
	void HandleDebugRoom7();
	UFUNCTION()
	void HandleDebugReset();
	UFUNCTION()
	void HandleDebugJoystick();
};

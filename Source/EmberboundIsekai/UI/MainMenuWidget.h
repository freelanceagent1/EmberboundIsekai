#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class EMBERBOUNDISEKAI_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

private:
	UPROPERTY()
	TObjectPtr<UTextBlock> SettingsText;

	UButton* MakeMenuButton(const FText& Label, FName Name);
	void RefreshSettingsText();

	UFUNCTION()
	void HandlePlay();
	UFUNCTION()
	void HandleSettings();
	UFUNCTION()
	void HandleQuit();
};

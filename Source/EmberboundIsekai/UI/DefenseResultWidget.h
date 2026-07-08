#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ResourceTypes.h"
#include "DefenseResultWidget.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;

UCLASS()
class EMBERBOUNDISEKAI_API UDefenseResultWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	void SetResult(bool bVictory, const TMap<EEmberResource, int32>& Rewards);

private:
	UPROPERTY()
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY()
	TObjectPtr<UVerticalBox> RewardLines;

	UPROPERTY()
	TObjectPtr<UButton> CloseButton;

	UFUNCTION()
	void HandleClose();
};

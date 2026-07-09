#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnboardingHintWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class EMBERBOUNDISEKAI_API UOnboardingHintWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	static void ShowCards(const UObject* WorldContextObject, bool bExpeditionCards);

	void SetCards(const TArray<FText>& InCards, bool bMarkSeenOnComplete);

private:
	UPROPERTY()
	TObjectPtr<UTextBlock> BodyText;

	UPROPERTY()
	TObjectPtr<UTextBlock> StepText;

	UPROPERTY()
	TObjectPtr<UButton> NextButton;

	TArray<FText> Cards;
	int32 CurrentIndex = 0;
	bool bMarkSeen = false;

	void Refresh();

	UFUNCTION()
	void HandleNext();
};

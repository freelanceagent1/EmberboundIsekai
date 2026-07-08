#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/EmberGameInstance.h"
#include "RunResultWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class EMBERBOUNDISEKAI_API URunResultWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	void SetResult(const FEmberRunResult& Result);

private:
	UPROPERTY()
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY()
	TObjectPtr<UTextBlock> LootText;

	UPROPERTY()
	TObjectPtr<UButton> RunAgainButton;

	UPROPERTY()
	TObjectPtr<UButton> ReturnCampButton;

	UFUNCTION()
	void HandleRunAgain();

	UFUNCTION()
	void HandleReturnCamp();
};

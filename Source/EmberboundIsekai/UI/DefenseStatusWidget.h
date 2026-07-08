#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DefenseStatusWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class EMBERBOUNDISEKAI_API UDefenseStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	void SetStatus(int32 Wave, float CurrentHP, float MaxHP);

private:
	UPROPERTY()
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY()
	TObjectPtr<UProgressBar> CampfireHealthBar;
};

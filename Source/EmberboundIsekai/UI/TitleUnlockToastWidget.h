#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TitleUnlockToastWidget.generated.h"

class UTextBlock;

UCLASS()
class EMBERBOUNDISEKAI_API UTitleUnlockToastWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	void SetTitleId(FName TitleId);

private:
	UPROPERTY()
	TObjectPtr<UTextBlock> ToastText;
};

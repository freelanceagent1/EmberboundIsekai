#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MonsterUnlockToastWidget.generated.h"

class UTextBlock;

UCLASS()
class EMBERBOUNDISEKAI_API UMonsterUnlockToastWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	void SetMonsterId(FName MonsterId);

private:
	UPROPERTY()
	TObjectPtr<UTextBlock> ToastText;
};

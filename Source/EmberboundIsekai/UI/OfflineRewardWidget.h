#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/MonsterCollectionSubsystem.h"
#include "OfflineRewardWidget.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;

UCLASS()
class EMBERBOUNDISEKAI_API UOfflineRewardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	void SetReward(const FOfflineRewardResult& InReward);

private:
	UPROPERTY()
	TObjectPtr<UVerticalBox> RootBox;

	UPROPERTY()
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY()
	TObjectPtr<UVerticalBox> RewardLines;

	UPROPERTY()
	TObjectPtr<UButton> CollectButton;

	UFUNCTION()
	void HandleCollect();
};

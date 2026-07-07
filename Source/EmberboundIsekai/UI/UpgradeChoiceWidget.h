#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UpgradeChoiceWidget.generated.h"

class UButton;
class UHorizontalBox;
class URunUpgradeData;
class URunUpgradeManager;

UCLASS()
class EMBERBOUNDISEKAI_API UUpgradeChoiceWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	void SetManager(URunUpgradeManager* InManager) { Manager = InManager; }
	void SetOffer(const TArray<URunUpgradeData*>& InOffer);
	void ChooseIndex(int32 Index);
	int32 GetOfferCount() const { return Offer.Num(); }

private:
	UPROPERTY()
	TObjectPtr<URunUpgradeManager> Manager;

	UPROPERTY()
	TArray<TObjectPtr<URunUpgradeData>> Offer;

	UPROPERTY()
	TObjectPtr<UHorizontalBox> CardRow;

	UPROPERTY()
	TArray<TObjectPtr<UButton>> CardButtons;

	UFUNCTION()
	void ChooseFirst();

	UFUNCTION()
	void ChooseSecond();

	UFUNCTION()
	void ChooseThird();

	void RebuildCards();
	FSlateColor GetRarityColor(URunUpgradeData* Upgrade) const;
};

#include "UI/UpgradeChoiceWidget.h"
#include "Data/RunUpgradeData.h"
#include "Generated/EmberGeneratedArt.h"
#include "Run/RunUpgradeManager.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"
#include "Input/Reply.h"

void UUpgradeChoiceWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetIsFocusable(true);
	CardRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("CardRow"));
	WidgetTree->RootWidget = CardRow;
}

FReply UUpgradeChoiceWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::One || Key == EKeys::NumPadOne)
	{
		ChooseIndex(0);
		return FReply::Handled();
	}
	if (Key == EKeys::Two || Key == EKeys::NumPadTwo)
	{
		ChooseIndex(1);
		return FReply::Handled();
	}
	if (Key == EKeys::Three || Key == EKeys::NumPadThree)
	{
		ChooseIndex(2);
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UUpgradeChoiceWidget::SetOffer(const TArray<URunUpgradeData*>& InOffer)
{
	Offer.Reset();
	for (URunUpgradeData* Upgrade : InOffer)
	{
		Offer.Add(Upgrade);
	}
	RebuildCards();
}

void UUpgradeChoiceWidget::ChooseIndex(int32 Index)
{
	if (!Offer.IsValidIndex(Index) || !Manager)
	{
		return;
	}
	URunUpgradeData* Picked = Offer[Index];
	Manager->ApplyUpgrade(Picked);
	RemoveFromParent();
	Manager->NotifyChoiceClosed();
}

void UUpgradeChoiceWidget::ChooseFirst()
{
	ChooseIndex(0);
}

void UUpgradeChoiceWidget::ChooseSecond()
{
	ChooseIndex(1);
}

void UUpgradeChoiceWidget::ChooseThird()
{
	ChooseIndex(2);
}

void UUpgradeChoiceWidget::RebuildCards()
{
	if (!CardRow)
	{
		return;
	}
	CardRow->ClearChildren();
	CardButtons.Reset();
	for (int32 Index = 0; Index < Offer.Num(); ++Index)
	{
		URunUpgradeData* Upgrade = Offer[Index];
		UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		UVerticalBox* Body = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		UTextBlock* Description = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		UTextBlock* Rarity = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		UImage* Icon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		Title->SetText(Upgrade ? Upgrade->Title : FText::GetEmpty());
		Title->SetColorAndOpacity(GetRarityColor(Upgrade));
		Description->SetText(Upgrade ? Upgrade->Description : FText::GetEmpty());
		Description->SetAutoWrapText(true);
		const FString RarityText = Upgrade ? StaticEnum<ERunUpgradeRarity>()->GetNameStringByValue(static_cast<int64>(Upgrade->Rarity)) : FString();
		Rarity->SetText(FText::FromString(FString::Printf(TEXT("%d  %s"), Index + 1, *RarityText)));
		Rarity->SetColorAndOpacity(GetRarityColor(Upgrade));
		if (Upgrade)
		{
			if (UTexture2D* Texture = Upgrade->Icon.LoadSynchronous())
			{
				Icon->SetBrushFromTexture(Texture, true);
			}
			else if (UTexture2D* ConventionTexture = EmberGeneratedArt::LoadTexture(EmberGeneratedArt::UpgradeIconTexturePath(Upgrade->Id)))
			{
				Icon->SetBrushFromTexture(ConventionTexture, true);
			}
		}
		Body->AddChildToVerticalBox(Rarity);
		Body->AddChildToVerticalBox(Icon);
		Body->AddChildToVerticalBox(Title);
		Body->AddChildToVerticalBox(Description);
		Button->AddChild(Body);
		if (Index == 0)
		{
			Button->OnClicked.AddDynamic(this, &UUpgradeChoiceWidget::ChooseFirst);
		}
		else if (Index == 1)
		{
			Button->OnClicked.AddDynamic(this, &UUpgradeChoiceWidget::ChooseSecond);
		}
		else
		{
			Button->OnClicked.AddDynamic(this, &UUpgradeChoiceWidget::ChooseThird);
		}
		UHorizontalBoxSlot* CardSlot = CardRow->AddChildToHorizontalBox(Button);
		CardSlot->SetPadding(FMargin(18.0f));
		CardSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		CardButtons.Add(Button);
	}
	SetKeyboardFocus();
}

FSlateColor UUpgradeChoiceWidget::GetRarityColor(URunUpgradeData* Upgrade) const
{
	if (!Upgrade)
	{
		return FSlateColor(FLinearColor::White);
	}
	switch (Upgrade->Rarity)
	{
	case ERunUpgradeRarity::Rare:
		return FSlateColor(FLinearColor(0.25f, 0.55f, 1.0f));
	case ERunUpgradeRarity::Epic:
		return FSlateColor(FLinearColor(0.85f, 0.35f, 1.0f));
	default:
		return FSlateColor(FLinearColor(0.85f, 0.9f, 0.82f));
	}
}

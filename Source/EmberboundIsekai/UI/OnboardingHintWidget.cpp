#include "UI/OnboardingHintWidget.h"

#include "Core/EmberSaveGame.h"
#include "Core/SaveManagerSubsystem.h"
#include "UI/EmberUIStyle.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "EmberboundIsekai.h"

void UOnboardingHintWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("OnboardingRoot"));
	WidgetTree->RootWidget = Root;

	UBorder* Card = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("HintCard"));
	EmberUIStyle::ApplyPanel(Card);
	UOverlaySlot* CardSlot = Root->AddChildToOverlay(Card);
	CardSlot->SetHorizontalAlignment(HAlign_Center);
	CardSlot->SetVerticalAlignment(VAlign_Bottom);
	CardSlot->SetPadding(FMargin(24.0f, 24.0f, 24.0f, 42.0f));

	UVerticalBox* Stack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("HintStack"));
	Card->SetContent(Stack);

	StepText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HintStep"));
	EmberUIStyle::ApplyText(StepText, 15.0f, EmberUIStyle::Ember());
	Stack->AddChildToVerticalBox(StepText);

	BodyText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HintBody"));
	BodyText->SetAutoWrapText(true);
	EmberUIStyle::ApplyText(BodyText, 22.0f);
	Stack->AddChildToVerticalBox(BodyText);

	NextButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("NextHint"));
	EmberUIStyle::ApplyButton(NextButton);
	UTextBlock* NextText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NextHintText"));
	NextText->SetText(NSLOCTEXT("Onboarding", "Next", "GOT IT"));
	NextText->SetJustification(ETextJustify::Center);
	EmberUIStyle::ApplyText(NextText, 18.0f, EmberUIStyle::Bone(), true);
	NextButton->AddChild(NextText);
	NextButton->OnClicked.AddDynamic(this, &UOnboardingHintWidget::HandleNext);
	Stack->AddChildToVerticalBox(NextButton)->SetPadding(FMargin(0.0f, 12.0f, 0.0f, 0.0f));
}

void UOnboardingHintWidget::ShowCards(const UObject* WorldContextObject, bool bExpeditionCards)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	USaveManagerSubsystem* Save = PC && PC->GetGameInstance() ? PC->GetGameInstance()->GetSubsystem<USaveManagerSubsystem>() : nullptr;
	const UEmberSaveGame* CurrentSave = Save ? Save->GetCurrentSave() : nullptr;
	if (!PC || !CurrentSave || CurrentSave->bSeenTutorial)
	{
		return;
	}

	UOnboardingHintWidget* Widget = CreateWidget<UOnboardingHintWidget>(PC, UOnboardingHintWidget::StaticClass());
	if (!Widget)
	{
		return;
	}

	TArray<FText> Cards;
	if (bExpeditionCards)
	{
		Cards = {
			NSLOCTEXT("Onboarding", "Move", "Drag the left side of the screen to move - you attack automatically."),
			NSLOCTEXT("Onboarding", "ClearRoom", "Clear each room; on level-up pick 1 of 3 upgrades."),
			NSLOCTEXT("Onboarding", "BossGoal", "Reach Room 7 and defeat the Tutorial Dragon.")
		};
	}
	else
	{
		Cards = {
			NSLOCTEXT("Onboarding", "CampExpedition", "Tap EXPEDITION to enter the dungeon."),
			NSLOCTEXT("Onboarding", "CampSpend", "Back at camp, spend Gold, Wood, Soul Ash, and Embers to grow stronger.")
		};
	}
	Widget->SetCards(Cards, bExpeditionCards);
	Widget->AddToViewport(900);
}

void UOnboardingHintWidget::SetCards(const TArray<FText>& InCards, bool bMarkSeenOnComplete)
{
	Cards = InCards;
	bMarkSeen = bMarkSeenOnComplete;
	CurrentIndex = 0;
	Refresh();
}

void UOnboardingHintWidget::Refresh()
{
	if (!Cards.IsValidIndex(CurrentIndex))
	{
		return;
	}
	StepText->SetText(FText::Format(NSLOCTEXT("Onboarding", "StepFmt", "Hint {0}/{1}"), CurrentIndex + 1, Cards.Num()));
	BodyText->SetText(Cards[CurrentIndex]);
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: Onboarding step=%d"), CurrentIndex + 1);
}

void UOnboardingHintWidget::HandleNext()
{
	++CurrentIndex;
	if (Cards.IsValidIndex(CurrentIndex))
	{
		Refresh();
		return;
	}
	if (bMarkSeen)
	{
		if (USaveManagerSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<USaveManagerSubsystem>() : nullptr)
		{
			if (UEmberSaveGame* CurrentSave = Save->GetCurrentSave())
			{
				CurrentSave->bSeenTutorial = true;
				Save->SaveNow();
			}
		}
	}
	RemoveFromParent();
}

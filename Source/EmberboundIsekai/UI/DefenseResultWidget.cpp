#include "UI/DefenseResultWidget.h"
#include "UI/EmberUIStyle.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

namespace
{
	FString DefenseResourceLabel(EEmberResource Resource)
	{
		switch (Resource)
		{
		case EEmberResource::Gold: return TEXT("Gold");
		case EEmberResource::Wood: return TEXT("Wood");
		case EEmberResource::SoulAsh: return TEXT("Soul Ash");
		case EEmberResource::MonsterEssence: return TEXT("Essence");
		case EEmberResource::Embers: return TEXT("Embers");
		default: return TEXT("Resource");
		}
	}
}

void UDefenseResultWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DefenseResultPanel"));
	EmberUIStyle::ApplyPanel(Panel);
	UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Root"));
	Panel->SetContent(Root);
	WidgetTree->RootWidget = Panel;
	TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Title"));
	RewardLines = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RewardLines"));
	CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CloseButton"));
	UTextBlock* CloseText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CloseText"));
	CloseText->SetText(NSLOCTEXT("Defense", "Close", "Close"));
	EmberUIStyle::ApplyText(CloseText, 18.0f, EmberUIStyle::Bone(), true);
	EmberUIStyle::ApplyButton(CloseButton);
	CloseButton->AddChild(CloseText);
	CloseButton->OnClicked.AddDynamic(this, &UDefenseResultWidget::HandleClose);
	Root->AddChildToVerticalBox(TitleText);
	Root->AddChildToVerticalBox(RewardLines);
	Root->AddChildToVerticalBox(CloseButton);
	EmberUIStyle::ApplyText(TitleText, 24.0f, EmberUIStyle::Ember(), true);
}

void UDefenseResultWidget::SetResult(bool bVictory, const TMap<EEmberResource, int32>& Rewards)
{
	if (TitleText)
	{
		TitleText->SetText(bVictory ? NSLOCTEXT("Defense", "Victory", "Defense Victory") : NSLOCTEXT("Defense", "Defeat", "Defense Defeat"));
	}
	if (!RewardLines)
	{
		return;
	}
	RewardLines->ClearChildren();
	for (const TPair<EEmberResource, int32>& Pair : Rewards)
	{
		UTextBlock* Line = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Line->SetText(FText::FromString(FString::Printf(TEXT("%s: +%d"), *DefenseResourceLabel(Pair.Key), Pair.Value)));
		EmberUIStyle::ApplyText(Line, 17.0f);
		RewardLines->AddChildToVerticalBox(Line);
	}
}

void UDefenseResultWidget::HandleClose()
{
	RemoveFromParent();
}

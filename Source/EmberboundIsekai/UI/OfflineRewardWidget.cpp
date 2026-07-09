#include "UI/OfflineRewardWidget.h"
#include "UI/EmberUIStyle.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Data/ResourceTypes.h"
#include "EmberboundIsekai.h"

namespace
{
	FString ResourceLabel(EEmberResource Resource)
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

void UOfflineRewardWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("OfflinePanel"));
	EmberUIStyle::ApplyPanel(Panel);
	RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Root"));
	Panel->SetContent(RootBox);
	WidgetTree->RootWidget = Panel;
	TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Title"));
	RewardLines = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RewardLines"));
	CollectButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CollectButton"));
	UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CollectText"));
	ButtonText->SetText(NSLOCTEXT("OfflineReward", "Collect", "Collect"));
	EmberUIStyle::ApplyText(ButtonText, 18.0f, EmberUIStyle::Bone(), true);
	EmberUIStyle::ApplyButton(CollectButton);
	CollectButton->AddChild(ButtonText);
	CollectButton->OnClicked.AddDynamic(this, &UOfflineRewardWidget::HandleCollect);
	RootBox->AddChildToVerticalBox(TitleText);
	RootBox->AddChildToVerticalBox(RewardLines);
	RootBox->AddChildToVerticalBox(CollectButton);
	EmberUIStyle::ApplyText(TitleText, 24.0f, EmberUIStyle::Ember(), true);
}

void UOfflineRewardWidget::SetReward(const FOfflineRewardResult& InReward)
{
	if (!TitleText || !RewardLines)
	{
		return;
	}
	TitleText->SetText(FText::FromString(FString::Printf(TEXT("Offline rewards: %d min%s"), InReward.Minutes, InReward.bCapped ? TEXT(" capped") : TEXT(""))));
	RewardLines->ClearChildren();
	for (const TPair<EEmberResource, int32>& Pair : InReward.Rewards)
	{
		UTextBlock* Line = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Line->SetText(FText::FromString(FString::Printf(TEXT("%s: +%d"), *ResourceLabel(Pair.Key), Pair.Value)));
		EmberUIStyle::ApplyText(Line, 17.0f);
		RewardLines->AddChildToVerticalBox(Line);
	}
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: OfflineRewardPopup lines=%d"), InReward.Rewards.Num());
}

void UOfflineRewardWidget::HandleCollect()
{
	RemoveFromParent();
}

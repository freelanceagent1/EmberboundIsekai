#include "UI/OfflineRewardWidget.h"
#include "Blueprint/WidgetTree.h"
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
	RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = RootBox;
	TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Title"));
	RewardLines = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RewardLines"));
	CollectButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CollectButton"));
	UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CollectText"));
	ButtonText->SetText(NSLOCTEXT("OfflineReward", "Collect", "Collect"));
	CollectButton->AddChild(ButtonText);
	CollectButton->OnClicked.AddDynamic(this, &UOfflineRewardWidget::HandleCollect);
	RootBox->AddChildToVerticalBox(TitleText);
	RootBox->AddChildToVerticalBox(RewardLines);
	RootBox->AddChildToVerticalBox(CollectButton);
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
		RewardLines->AddChildToVerticalBox(Line);
	}
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: OfflineRewardPopup lines=%d"), InReward.Rewards.Num());
}

void UOfflineRewardWidget::HandleCollect()
{
	RemoveFromParent();
}

#include "UI/RunResultWidget.h"
#include "Core/SceneFlowSubsystem.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"

void URunResultWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
	LootText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LootText"));
	RunAgainButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RunAgainButton"));
	ReturnCampButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ReturnCampButton"));

	UTextBlock* RunAgainText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RunAgainText"));
	RunAgainText->SetText(NSLOCTEXT("RunResult", "RunAgain", "Run Again"));
	RunAgainButton->AddChild(RunAgainText);
	RunAgainButton->OnClicked.AddDynamic(this, &URunResultWidget::HandleRunAgain);

	UTextBlock* ReturnText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ReturnCampText"));
	ReturnText->SetText(NSLOCTEXT("RunResult", "ReturnCamp", "Return to Camp"));
	ReturnCampButton->AddChild(ReturnText);
	ReturnCampButton->SetIsEnabled(true);
	ReturnCampButton->OnClicked.AddDynamic(this, &URunResultWidget::HandleReturnCamp);

	Root->AddChildToVerticalBox(TitleText);
	Root->AddChildToVerticalBox(LootText);
	Root->AddChildToVerticalBox(RunAgainButton);
	Root->AddChildToVerticalBox(ReturnCampButton);
}

void URunResultWidget::SetResult(const FEmberRunResult& Result)
{
	TitleText->SetText(Result.bVictory ? NSLOCTEXT("RunResult", "Victory", "VICTORY") : NSLOCTEXT("RunResult", "Defeat", "DEFEAT"));

	TArray<FString> Lines;
	const auto AddResourceLine = [&Lines, &Result](EEmberResource Resource, const TCHAR* Label)
	{
		if (const int32* Amount = Result.BankedResources.Find(Resource))
		{
			if (*Amount > 0)
			{
				Lines.Add(FString::Printf(TEXT("%s: %d"), Label, *Amount));
			}
		}
	};
	AddResourceLine(EEmberResource::Gold, TEXT("Gold"));
	AddResourceLine(EEmberResource::Wood, TEXT("Wood"));
	AddResourceLine(EEmberResource::SoulAsh, TEXT("Soul Ash"));
	AddResourceLine(EEmberResource::Embers, TEXT("Embers"));
	for (const TPair<FName, int32>& Shard : Result.MonsterShards)
	{
		Lines.Add(FString::Printf(TEXT("%s Shards: %d"), *Shard.Key.ToString(), Shard.Value));
	}
	if (Lines.Num() == 0)
	{
		Lines.Add(TEXT("No banked loot"));
	}
	LootText->SetText(FText::FromString(FString::Join(Lines, TEXT("\n"))));
}

void URunResultWidget::HandleRunAgain()
{
	UGameplayStatics::OpenLevel(this, TEXT("/Game/Emberbound/Maps/Expedition/L_Expedition"), true, TEXT("game=/Script/EmberboundIsekai.ExpeditionGameMode"));
}

void URunResultWidget::HandleReturnCamp()
{
	if (USceneFlowSubsystem* Flow = GetGameInstance() ? GetGameInstance()->GetSubsystem<USceneFlowSubsystem>() : nullptr)
	{
		Flow->TravelToCamp();
	}
}

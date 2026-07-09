#include "UI/DefenseStatusWidget.h"
#include "UI/EmberUIStyle.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

void UDefenseStatusWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DefenseStatusPanel"));
	EmberUIStyle::ApplyPanel(Panel, FLinearColor(0.86f, 0.72f, 0.49f, 0.88f));
	UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Root"));
	Panel->SetContent(Root);
	WidgetTree->RootWidget = Panel;
	TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DefenseTitle"));
	CampfireHealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("CampfireHealth"));
	Root->AddChildToVerticalBox(TitleText);
	Root->AddChildToVerticalBox(CampfireHealthBar);
	EmberUIStyle::ApplyText(TitleText, 18.0f, EmberUIStyle::Ink());
	SetStatus(0, 1.0f, 1.0f);
}

void UDefenseStatusWidget::SetStatus(int32 Wave, float CurrentHP, float MaxHP)
{
	if (TitleText)
	{
		TitleText->SetText(FText::FromString(FString::Printf(TEXT("Defense Wave %d/4 - Campfire %.0f/%.0f"), FMath::Max(1, Wave), CurrentHP, MaxHP)));
	}
	if (CampfireHealthBar)
	{
		CampfireHealthBar->SetPercent(MaxHP > 0.0f ? CurrentHP / MaxHP : 0.0f);
	}
}

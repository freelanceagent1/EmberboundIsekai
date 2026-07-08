#include "UI/TitleUnlockToastWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

void UTitleUnlockToastWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;
	ToastText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ToastText"));
	Root->AddChildToVerticalBox(ToastText);
}

void UTitleUnlockToastWidget::SetTitleId(FName TitleId)
{
	if (ToastText)
	{
		ToastText->SetText(FText::FromString(FString::Printf(TEXT("Title unlocked: %s"), *TitleId.ToString())));
	}
}

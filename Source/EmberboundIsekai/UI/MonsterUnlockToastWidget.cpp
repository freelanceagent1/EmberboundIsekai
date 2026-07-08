#include "UI/MonsterUnlockToastWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

void UMonsterUnlockToastWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;
	ToastText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ToastText"));
	Root->AddChildToVerticalBox(ToastText);
}

void UMonsterUnlockToastWidget::SetMonsterId(FName MonsterId)
{
	if (ToastText)
	{
		ToastText->SetText(FText::FromString(FString::Printf(TEXT("Monster unlocked: %s"), *MonsterId.ToString())));
	}
}

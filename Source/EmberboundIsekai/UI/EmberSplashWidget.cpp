#include "UI/EmberSplashWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Generated/EmberGeneratedArt.h"
#include "TimerManager.h"

void UEmberSplashWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetIsFocusable(true);
	UImage* SplashImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("SplashImage"));
	WidgetTree->RootWidget = SplashImage;
	if (UTexture2D* SplashTexture = EmberGeneratedArt::LoadTexture(TEXT("/Game/Emberbound/Generated/KeyArt/T_MainSplash.T_MainSplash")))
	{
		SplashImage->SetBrushFromTexture(SplashTexture, true);
	}
}

void UEmberSplashWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetKeyboardFocus();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(AutoDismissTimerHandle, this, &UEmberSplashWidget::Dismiss, 2.0f, false);
	}
}

FReply UEmberSplashWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	Dismiss();
	return FReply::Handled();
}

FReply UEmberSplashWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Dismiss();
	return FReply::Handled();
}

void UEmberSplashWidget::Dismiss()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoDismissTimerHandle);
	}
	RemoveFromParent();
}

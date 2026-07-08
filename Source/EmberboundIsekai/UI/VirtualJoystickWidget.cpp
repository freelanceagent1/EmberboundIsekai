#include "UI/VirtualJoystickWidget.h"
#include "Combat/EmberPlayerCharacter.h"
#include "Core/EmberSaveGame.h"
#include "Core/SaveManagerSubsystem.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<int32> CVarEmberShowJoystick(
	TEXT("Ember.ShowJoystick"),
	0,
	TEXT("Show the native Ember virtual joystick for desktop/mobile validation."));

bool UVirtualJoystickWidget::ShouldShowJoystick(const UObject* WorldContextObject)
{
	if (CVarEmberShowJoystick.GetValueOnGameThread() != 0 || FPlatformMisc::SupportsTouchInput())
	{
		return true;
	}
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	const USaveManagerSubsystem* SaveManager = World && World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<USaveManagerSubsystem>() : nullptr;
	return SaveManager && SaveManager->GetCurrentSave() && SaveManager->GetCurrentSave()->bShowJoystick;
}

void UVirtualJoystickWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("JoystickRoot"));
	WidgetTree->RootWidget = Root;

	USizeBox* RingBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("JoystickRingBox"));
	RingBox->SetWidthOverride(180.0f);
	RingBox->SetHeightOverride(180.0f);
	UBorder* Ring = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("JoystickRing"));
	Ring->SetBrushColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.18f));
	RingBox->AddChild(Ring);
	UCanvasPanelSlot* RingSlot = Root->AddChildToCanvas(RingBox);
	RingSlot->SetAnchors(FAnchors(0.0f, 1.0f));
	RingSlot->SetAlignment(FVector2D(0.0f, 1.0f));
	RingSlot->SetPosition(FVector2D(48.0f, -48.0f));
	RingSlot->SetSize(FVector2D(180.0f, 180.0f));

	USizeBox* ThumbBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("JoystickThumbBox"));
	ThumbBox->SetWidthOverride(72.0f);
	ThumbBox->SetHeightOverride(72.0f);
	Thumb = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("JoystickThumb"));
	Thumb->SetBrushColor(FLinearColor(0.95f, 0.75f, 0.25f, 0.55f));
	ThumbBox->AddChild(Thumb);
	UCanvasPanelSlot* ThumbSlot = Root->AddChildToCanvas(ThumbBox);
	ThumbSlot->SetAnchors(FAnchors(0.0f, 1.0f));
	ThumbSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	ThumbSlot->SetPosition(FVector2D(138.0f, -138.0f));
	ThumbSlot->SetSize(FVector2D(72.0f, 72.0f));
}

void UVirtualJoystickWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (bDragging)
	{
		PushMovement();
	}
}

FReply UVirtualJoystickWidget::NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	bDragging = true;
	UpdateFromScreenPosition(InGeometry, InGestureEvent.GetScreenSpacePosition());
	return FReply::Handled();
}

FReply UVirtualJoystickWidget::NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	UpdateFromScreenPosition(InGeometry, InGestureEvent.GetScreenSpacePosition());
	return FReply::Handled();
}

FReply UVirtualJoystickWidget::NativeOnTouchEnded(const FGeometry&, const FPointerEvent&)
{
	StopDrag();
	return FReply::Handled();
}

FReply UVirtualJoystickWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	bDragging = true;
	UpdateFromScreenPosition(InGeometry, InMouseEvent.GetScreenSpacePosition());
	return FReply::Handled().CaptureMouse(TakeWidget());
}

FReply UVirtualJoystickWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bDragging)
	{
		UpdateFromScreenPosition(InGeometry, InMouseEvent.GetScreenSpacePosition());
		return FReply::Handled();
	}
	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

FReply UVirtualJoystickWidget::NativeOnMouseButtonUp(const FGeometry&, const FPointerEvent&)
{
	StopDrag();
	return FReply::Handled().ReleaseMouseCapture();
}

void UVirtualJoystickWidget::UpdateFromScreenPosition(const FGeometry& InGeometry, const FVector2D& ScreenPosition)
{
	const FVector2D Local = InGeometry.AbsoluteToLocal(ScreenPosition);
	const FVector2D Center(138.0f, InGeometry.GetLocalSize().Y - 138.0f);
	const FVector2D Delta = Local - Center;
	CurrentVector = FVector2D(Delta.X, -Delta.Y).GetClampedToMaxSize(90.0f) / 90.0f;
	PushMovement();
}

void UVirtualJoystickWidget::StopDrag()
{
	bDragging = false;
	CurrentVector = FVector2D::ZeroVector;
}

void UVirtualJoystickWidget::PushMovement() const
{
	if (AEmberPlayerCharacter* Player = Cast<AEmberPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		Player->ApplyVirtualJoystickVector(CurrentVector);
	}
}

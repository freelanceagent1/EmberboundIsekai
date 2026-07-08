#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VirtualJoystickWidget.generated.h"

class AEmberPlayerCharacter;
class UBorder;

UCLASS()
class EMBERBOUNDISEKAI_API UVirtualJoystickWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	static bool ShouldShowJoystick(const UObject* WorldContextObject);

	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	virtual FReply NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	virtual FReply NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	UPROPERTY()
	TObjectPtr<UBorder> Thumb;

	bool bDragging = false;
	FVector2D CurrentVector = FVector2D::ZeroVector;

	void UpdateFromScreenPosition(const FGeometry& InGeometry, const FVector2D& ScreenPosition);
	void StopDrag();
	void PushMovement() const;
};

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateTypes.h"

class UBorder;
class UButton;
class UTextBlock;

namespace EmberUIStyle
{
	inline FLinearColor Parchment() { return FLinearColor(0.86f, 0.72f, 0.49f, 0.94f); }
	inline FLinearColor ParchmentDark() { return FLinearColor(0.30f, 0.18f, 0.10f, 0.92f); }
	inline FLinearColor Ember() { return FLinearColor(1.0f, 0.35f, 0.08f, 1.0f); }
	inline FLinearColor Bone() { return FLinearColor(0.98f, 0.89f, 0.72f, 1.0f); }
	inline FLinearColor Ink() { return FLinearColor(0.12f, 0.07f, 0.035f, 1.0f); }

	void ApplyText(UTextBlock* Text, float Size, const FLinearColor& Color = Ink(), bool bShadow = false);
	void ApplyButton(UButton* Button);
	void ApplyPanel(UBorder* Border, const FLinearColor& Color = Parchment());
}

#include "UI/EmberUIStyle.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

namespace EmberUIStyle
{
	void ApplyText(UTextBlock* Text, float Size, const FLinearColor& Color, bool bShadow)
	{
		if (!Text)
		{
			return;
		}
		FSlateFontInfo Font = Text->GetFont();
		Font.Size = FMath::RoundToInt(Size);
		Text->SetFont(Font);
		Text->SetColorAndOpacity(FSlateColor(Color));
		if (bShadow)
		{
			Text->SetShadowOffset(FVector2D(2.0f, 2.0f));
			Text->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.55f));
		}
	}

	void ApplyButton(UButton* Button)
	{
		if (!Button)
		{
			return;
		}
		FButtonStyle Style = Button->GetStyle();
		Style.Normal.TintColor = FSlateColor(Ember());
		Style.Hovered.TintColor = FSlateColor(FLinearColor(1.0f, 0.52f, 0.15f, 1.0f));
		Style.Pressed.TintColor = FSlateColor(FLinearColor(0.75f, 0.18f, 0.04f, 1.0f));
		Style.Disabled.TintColor = FSlateColor(FLinearColor(0.23f, 0.18f, 0.14f, 0.6f));
		Style.NormalPadding = FMargin(18.0f, 10.0f);
		Style.PressedPadding = FMargin(18.0f, 12.0f, 18.0f, 8.0f);
		Button->SetStyle(Style);
	}

	void ApplyPanel(UBorder* Border, const FLinearColor& Color)
	{
		if (!Border)
		{
			return;
		}
		Border->SetBrushColor(Color);
		Border->SetPadding(FMargin(18.0f, 14.0f));
	}
}

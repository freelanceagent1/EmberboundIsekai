#include "UI/MainMenuWidget.h"

#include "Core/EmberSaveGame.h"
#include "Core/EmberGameInstance.h"
#include "Core/SaveManagerSubsystem.h"
#include "Camp/CampGameMode.h"
#include "Generated/EmberGeneratedArt.h"
#include "UI/EmberUIStyle.h"
#include "UI/OnboardingHintWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EmberboundIsekai.h"

void UMainMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetIsFocusable(true);

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("MenuRoot"));
	WidgetTree->RootWidget = Root;

	UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MenuBackdrop"));
	Backdrop->SetBrushColor(FLinearColor(0.10f, 0.055f, 0.02f, 1.0f));
	if (UCanvasPanelSlot* BackdropSlot = Root->AddChildToCanvas(Backdrop))
	{
		BackdropSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		BackdropSlot->SetOffsets(FMargin(0.0f));
	}

	UImage* KeyArt = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("MainSplash"));
	if (UTexture2D* Texture = EmberGeneratedArt::LoadTexture(TEXT("/Game/Emberbound/Generated/KeyArt/T_MainSplash.T_MainSplash")))
	{
		KeyArt->SetBrushFromTexture(Texture, false);
	}
	if (UCanvasPanelSlot* KeyArtSlot = Root->AddChildToCanvas(KeyArt))
	{
		KeyArtSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		KeyArtSlot->SetOffsets(FMargin(0.0f));
	}

	UVerticalBox* Stack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MenuStack"));
	if (UCanvasPanelSlot* StackSlot = Root->AddChildToCanvas(Stack))
	{
		StackSlot->SetAnchors(FAnchors(0.0f, 0.5f, 0.0f, 0.5f));
		StackSlot->SetAlignment(FVector2D(0.0f, 0.5f));
		StackSlot->SetPosition(FVector2D(48.0f, 0.0f));
		StackSlot->SetSize(FVector2D(520.0f, 360.0f));
	}

	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Title"));
	Title->SetText(NSLOCTEXT("MainMenu", "Title", "Emberbound Isekai"));
	EmberUIStyle::ApplyText(Title, 48.0f, EmberUIStyle::Bone(), true);
	Stack->AddChildToVerticalBox(Title);

	UTextBlock* Subtitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Subtitle"));
	Subtitle->SetText(NSLOCTEXT("MainMenu", "Subtitle", "Build camp. Clear seven rooms. Slay the Tutorial Dragon."));
	EmberUIStyle::ApplyText(Subtitle, 20.0f, EmberUIStyle::Bone(), true);
	UVerticalBoxSlot* SubtitleSlot = Stack->AddChildToVerticalBox(Subtitle);
	SubtitleSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 24.0f));

	UButton* Play = MakeMenuButton(NSLOCTEXT("MainMenu", "Play", "PLAY"), TEXT("PlayButton"));
	UButton* Settings = MakeMenuButton(NSLOCTEXT("MainMenu", "Settings", "SETTINGS"), TEXT("SettingsButton"));
	UButton* Quit = MakeMenuButton(NSLOCTEXT("MainMenu", "Quit", "QUIT"), TEXT("QuitButton"));
	Play->OnClicked.AddDynamic(this, &UMainMenuWidget::HandlePlay);
	Settings->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleSettings);
	Quit->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleQuit);
	Stack->AddChildToVerticalBox(Play)->SetPadding(FMargin(0.0f, 6.0f));
	Stack->AddChildToVerticalBox(Settings)->SetPadding(FMargin(0.0f, 6.0f));
	Stack->AddChildToVerticalBox(Quit)->SetPadding(FMargin(0.0f, 6.0f));

	SettingsText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SettingsText"));
	SettingsText->SetAutoWrapText(true);
	EmberUIStyle::ApplyText(SettingsText, 16.0f, EmberUIStyle::Bone(), true);
	Stack->AddChildToVerticalBox(SettingsText)->SetPadding(FMargin(0.0f, 16.0f, 0.0f, 0.0f));
	RefreshSettingsText();
}

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetKeyboardFocus();
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: MainMenuShown"));
}

UButton* UMainMenuWidget::MakeMenuButton(const FText& Label, FName Name)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	EmberUIStyle::ApplyButton(Button);
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Text->SetText(Label);
	Text->SetJustification(ETextJustify::Center);
	EmberUIStyle::ApplyText(Text, 24.0f, EmberUIStyle::Bone(), true);
	Button->AddChild(Text);
	return Button;
}

void UMainMenuWidget::RefreshSettingsText()
{
	const USaveManagerSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<USaveManagerSubsystem>() : nullptr;
	const UEmberSaveGame* CurrentSave = Save ? Save->GetCurrentSave() : nullptr;
	const float Volume = CurrentSave ? CurrentSave->MasterVolume : 1.0f;
	const bool bJoystick = CurrentSave && CurrentSave->bShowJoystick;
	SettingsText->SetText(FText::FromString(FString::Printf(TEXT("Volume %.0f%%  |  Joystick %s\nTap SETTINGS to cycle volume and joystick."), Volume * 100.0f, bJoystick ? TEXT("ON") : TEXT("OFF"))));
}

void UMainMenuWidget::HandlePlay()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeGameAndUI());
		if (UEmberGameInstance* EmberGI = Cast<UEmberGameInstance>(GetGameInstance()))
		{
			EmberGI->SetMainMenuActive(false);
		}
		if (ACampGameMode* CampMode = GetWorld() ? GetWorld()->GetAuthGameMode<ACampGameMode>() : nullptr)
		{
			CampMode->StartCampAfterMenu();
		}
	}
	RemoveFromParent();
}

void UMainMenuWidget::HandleSettings()
{
	if (USaveManagerSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<USaveManagerSubsystem>() : nullptr)
	{
		if (UEmberSaveGame* CurrentSave = Save->GetCurrentSave())
		{
			CurrentSave->bShowJoystick = !CurrentSave->bShowJoystick;
			CurrentSave->MasterVolume = CurrentSave->MasterVolume >= 0.99f ? 0.6f : 1.0f;
			Save->SaveNow();
		}
	}
	RefreshSettingsText();
}

void UMainMenuWidget::HandleQuit()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

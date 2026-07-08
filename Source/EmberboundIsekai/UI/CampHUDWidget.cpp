#include "UI/CampHUDWidget.h"
#include "Camp/DefenseEventManager.h"
#include "Core/EconomySubsystem.h"
#include "Core/SaveManagerSubsystem.h"
#include "Core/SceneFlowSubsystem.h"
#include "Generated/EmberGeneratedArt.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"

void UCampHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;
	UHorizontalBox* Resources = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Resources"));
	Root->AddChildToVerticalBox(Resources);
	for (EEmberResource Resource : { EEmberResource::Gold, EEmberResource::Wood, EEmberResource::SoulAsh, EEmberResource::MonsterEssence, EEmberResource::Embers })
	{
		const FName IconId = Resource == EEmberResource::Gold ? TEXT("gold") : Resource == EEmberResource::Wood ? TEXT("wood") : Resource == EEmberResource::SoulAsh ? TEXT("soul-ash") : Resource == EEmberResource::MonsterEssence ? TEXT("monster-essence") : TEXT("embers");
		UImage* Icon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		if (UTexture2D* Texture = EmberGeneratedArt::LoadTexture(EmberGeneratedArt::HudIconTexturePath(IconId)))
		{
			Icon->SetBrushFromTexture(Texture, true);
		}
		Resources->AddChildToHorizontalBox(Icon);
		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		ResourceTexts.Add(Resource, Text);
		Resources->AddChildToHorizontalBox(Text);
	}
	UHorizontalBox* Buttons = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Buttons"));
	Root->AddChildToVerticalBox(Buttons);
	UButton* Expedition = MakeButton(NSLOCTEXT("Camp", "Expedition", "Expedition"), TEXT("ExpeditionButton"));
	UButton* Upgrades = MakeButton(NSLOCTEXT("Camp", "Upgrades", "Upgrades"), TEXT("UpgradesButton"));
	UButton* Monsters = MakeButton(NSLOCTEXT("Camp", "Monsters", "Monsters"), TEXT("MonstersButton"));
	UButton* Defense = MakeButton(NSLOCTEXT("Camp", "Defense", "Defense"), TEXT("DefenseButton"));
	UButton* Save = MakeButton(NSLOCTEXT("Camp", "Save", "Save"), TEXT("SaveButton"));
	UButton* Reset = MakeButton(NSLOCTEXT("Camp", "Reset", "Reset"), TEXT("ResetButton"));
	Expedition->OnClicked.AddDynamic(this, &UCampHUDWidget::HandleExpedition);
	Upgrades->OnClicked.AddDynamic(this, &UCampHUDWidget::HandleUpgrades);
	Monsters->OnClicked.AddDynamic(this, &UCampHUDWidget::HandleMonsters);
	Defense->OnClicked.AddDynamic(this, &UCampHUDWidget::HandleDefense);
	Save->OnClicked.AddDynamic(this, &UCampHUDWidget::HandleSave);
	Reset->OnClicked.AddDynamic(this, &UCampHUDWidget::HandleReset);
	Buttons->AddChildToHorizontalBox(Expedition);
	Buttons->AddChildToHorizontalBox(Upgrades);
	Buttons->AddChildToHorizontalBox(Monsters);
	Buttons->AddChildToHorizontalBox(Defense);
	Buttons->AddChildToHorizontalBox(Save);
	Buttons->AddChildToHorizontalBox(Reset);
	StubText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StubText"));
	Root->AddChildToVerticalBox(StubText);
	RefreshResources();
}

void UCampHUDWidget::RefreshResources()
{
	UEconomySubsystem* Economy = GetGameInstance() ? GetGameInstance()->GetSubsystem<UEconomySubsystem>() : nullptr;
	for (TPair<EEmberResource, TObjectPtr<UTextBlock>>& Pair : ResourceTexts)
	{
		const FString Label = Pair.Key == EEmberResource::Gold ? TEXT("Gold") : Pair.Key == EEmberResource::Wood ? TEXT("Wood") : Pair.Key == EEmberResource::SoulAsh ? TEXT("Soul Ash") : Pair.Key == EEmberResource::MonsterEssence ? TEXT("Essence") : TEXT("Embers");
		Pair.Value->SetText(FText::FromString(FString::Printf(TEXT("%s %d  "), *Label, Economy ? Economy->Get(Pair.Key) : 0)));
	}
}

UButton* UCampHUDWidget::MakeButton(const FText& Label, FName Name)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Text->SetText(Label);
	Button->AddChild(Text);
	return Button;
}

void UCampHUDWidget::HandleExpedition()
{
	if (USceneFlowSubsystem* Flow = GetGameInstance() ? GetGameInstance()->GetSubsystem<USceneFlowSubsystem>() : nullptr)
	{
		Flow->TravelToExpedition();
	}
}

void UCampHUDWidget::HandleUpgrades()
{
	StubText->SetText(NSLOCTEXT("Camp", "UpgradesStub", "Walk to a building to upgrade"));
}

void UCampHUDWidget::HandleMonsters()
{
	StubText->SetText(NSLOCTEXT("Camp", "MonstersStub", "M6"));
}

void UCampHUDWidget::HandleDefense()
{
	if (UDefenseEventManager* Defense = GetWorld() ? GetWorld()->GetSubsystem<UDefenseEventManager>() : nullptr)
	{
		Defense->StartDefense();
	}
	StubText->SetText(NSLOCTEXT("Camp", "DefenseStarted", "Defense running"));
}

void UCampHUDWidget::HandleSave()
{
	if (USaveManagerSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<USaveManagerSubsystem>() : nullptr)
	{
		Save->SaveNow();
	}
	RefreshResources();
}

void UCampHUDWidget::HandleReset()
{
	if (USaveManagerSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<USaveManagerSubsystem>() : nullptr)
	{
		Save->ResetDebug();
	}
	RefreshResources();
}

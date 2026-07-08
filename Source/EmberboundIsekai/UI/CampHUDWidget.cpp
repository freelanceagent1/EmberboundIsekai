#include "UI/CampHUDWidget.h"
#include "Camp/DefenseEventManager.h"
#include "Core/EconomySubsystem.h"
#include "Core/SaveManagerSubsystem.h"
#include "Core/SceneFlowSubsystem.h"
#include "Core/EmberSaveGame.h"
#include "Core/MonsterCollectionSubsystem.h"
#include "Core/TitleSubsystem.h"
#include "Generated/EmberGeneratedArt.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/SafeZone.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "HAL/IConsoleManager.h"
#include "EmberboundIsekai.h"

void UCampHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	USafeZone* SafeRoot = WidgetTree->ConstructWidget<USafeZone>(USafeZone::StaticClass(), TEXT("SafeRoot"));
	UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Root"));
	SafeRoot->AddChild(Root);
	WidgetTree->RootWidget = SafeRoot;
	UHorizontalBox* Resources = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Resources"));
	Root->AddChildToVerticalBox(Resources);
	int32 ResolvedIcons = 0;
	int32 ExpectedIcons = 0;
	for (EEmberResource Resource : { EEmberResource::Gold, EEmberResource::Wood, EEmberResource::SoulAsh, EEmberResource::MonsterEssence, EEmberResource::Embers })
	{
		++ExpectedIcons;
		const FName IconId = Resource == EEmberResource::Gold ? TEXT("gold") : Resource == EEmberResource::Wood ? TEXT("wood") : Resource == EEmberResource::SoulAsh ? TEXT("soul-ash") : Resource == EEmberResource::MonsterEssence ? TEXT("monster-essence") : TEXT("embers");
		UImage* Icon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		if (UTexture2D* Texture = EmberGeneratedArt::LoadTexture(EmberGeneratedArt::HudIconTexturePath(IconId)))
		{
			Icon->SetBrushFromTexture(Texture, false);
			Icon->SetDesiredSizeOverride(FVector2D(28.0f, 28.0f));
			++ResolvedIcons;
		}
		Resources->AddChildToHorizontalBox(Icon);
		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		ResourceTexts.Add(Resource, Text);
		Resources->AddChildToHorizontalBox(Text);
	}
	UE_LOG(LogEmberbound, Display, TEXT("EMBER: IconResolved %d/%d"), ResolvedIcons, ExpectedIcons);
	UHorizontalBox* Buttons = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Buttons"));
	Root->AddChildToVerticalBox(Buttons);
	UButton* Expedition = MakeButton(NSLOCTEXT("Camp", "Expedition", "Expedition"), TEXT("ExpeditionButton"));
	UButton* Upgrades = MakeButton(NSLOCTEXT("Camp", "Upgrades", "Upgrades"), TEXT("UpgradesButton"));
	UButton* Monsters = MakeButton(NSLOCTEXT("Camp", "Monsters", "Monsters"), TEXT("MonstersButton"));
	UButton* Titles = MakeButton(NSLOCTEXT("Camp", "Titles", "Titles"), TEXT("TitlesButton"));
	UButton* Defense = MakeButton(NSLOCTEXT("Camp", "Defense", "Defense"), TEXT("DefenseButton"));
	UButton* Save = MakeButton(NSLOCTEXT("Camp", "Save", "Save"), TEXT("SaveButton"));
	UButton* Reset = MakeButton(NSLOCTEXT("Camp", "Reset", "Reset"), TEXT("ResetButton"));
	Expedition->OnClicked.AddDynamic(this, &UCampHUDWidget::HandleExpedition);
	Upgrades->OnClicked.AddDynamic(this, &UCampHUDWidget::HandleUpgrades);
	Monsters->OnClicked.AddDynamic(this, &UCampHUDWidget::HandleMonsters);
	Titles->OnClicked.AddDynamic(this, &UCampHUDWidget::HandleTitles);
	Defense->OnClicked.AddDynamic(this, &UCampHUDWidget::HandleDefense);
	Save->OnClicked.AddDynamic(this, &UCampHUDWidget::HandleSave);
	Reset->OnClicked.AddDynamic(this, &UCampHUDWidget::HandleReset);
	Buttons->AddChildToHorizontalBox(Expedition);
	Buttons->AddChildToHorizontalBox(Upgrades);
	Buttons->AddChildToHorizontalBox(Monsters);
	Buttons->AddChildToHorizontalBox(Titles);
	Buttons->AddChildToHorizontalBox(Defense);
	Buttons->AddChildToHorizontalBox(Save);
	Buttons->AddChildToHorizontalBox(Reset);
#if !UE_BUILD_SHIPPING
	UButton* Debug = MakeButton(NSLOCTEXT("Camp", "Debug", "Debug"), TEXT("DebugButton"));
	Debug->OnClicked.AddDynamic(this, &UCampHUDWidget::HandleDebug);
	Buttons->AddChildToHorizontalBox(Debug);
#endif
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

void UCampHUDWidget::HandleTitles()
{
	UTitleSubsystem* Titles = GetGameInstance() ? GetGameInstance()->GetSubsystem<UTitleSubsystem>() : nullptr;
	if (!Titles)
	{
		return;
	}
	TArray<FString> Lines;
	Lines.Add(TEXT("Titles"));
	for (const UTitleData* Title : Titles->GetCatalog())
	{
		if (!Title)
		{
			continue;
		}
		Lines.Add(FString::Printf(TEXT("%s - %s - %.0f%%"),
			Titles->IsUnlocked(Title->Id) ? TEXT("Unlocked") : TEXT("Locked"),
			*Title->DisplayName.ToString(),
			Title->Passive.Magnitude * 100.0f));
	}
	StubText->SetText(FText::FromString(FString::Join(Lines, TEXT("\n"))));
}

void UCampHUDWidget::HandleDebug()
{
#if !UE_BUILD_SHIPPING
	StubText->SetText(NSLOCTEXT("Camp", "DebugPanel", "Debug: GrantResources | GrantShards | Offline120 | StartDefense | StartRoom7 | ResetSave | Joystick"));
	HandleDebugGrantResources();
#endif
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

void UCampHUDWidget::HandleDebugGrantResources()
{
	if (USaveManagerSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<USaveManagerSubsystem>() : nullptr)
	{
		Save->GrantResources(1000, 1000, 1000, 1000, 0);
	}
	RefreshResources();
}

void UCampHUDWidget::HandleDebugGrantShards()
{
	if (UMonsterCollectionSubsystem* Monsters = GetGameInstance() ? GetGameInstance()->GetSubsystem<UMonsterCollectionSubsystem>() : nullptr)
	{
		Monsters->GrantShards(TEXT("slime"), 5);
	}
}

void UCampHUDWidget::HandleDebugOffline()
{
	if (UMonsterCollectionSubsystem* Monsters = GetGameInstance() ? GetGameInstance()->GetSubsystem<UMonsterCollectionSubsystem>() : nullptr)
	{
		Monsters->SimulateOffline(120);
	}
}

void UCampHUDWidget::HandleDebugRoom7()
{
	if (IConsoleVariable* StartRoom = IConsoleManager::Get().FindConsoleVariable(TEXT("Ember.StartRoom")))
	{
		StartRoom->Set(7, ECVF_SetByConsole);
	}
	HandleExpedition();
}

void UCampHUDWidget::HandleDebugReset()
{
	HandleReset();
}

void UCampHUDWidget::HandleDebugJoystick()
{
	if (USaveManagerSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<USaveManagerSubsystem>() : nullptr)
	{
		if (UEmberSaveGame* CurrentSave = Save->GetCurrentSave())
		{
			CurrentSave->bShowJoystick = !CurrentSave->bShowJoystick;
			Save->SaveNow();
		}
	}
}

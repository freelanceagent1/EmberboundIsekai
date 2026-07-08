#include "UI/CampBuildingPanelWidget.h"
#include "Core/EconomySubsystem.h"
#include "Core/SaveManagerSubsystem.h"
#include "Data/BuildingData.h"
#include "Generated/EmberGeneratedArt.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"

static FString ResourceLabel(EEmberResource Resource)
{
	switch (Resource)
	{
	case EEmberResource::Gold: return TEXT("Gold");
	case EEmberResource::Wood: return TEXT("Wood");
	case EEmberResource::SoulAsh: return TEXT("Soul Ash");
	case EEmberResource::MonsterEssence: return TEXT("Essence");
	case EEmberResource::Embers: return TEXT("Embers");
	default: return TEXT("Resource");
	}
}

void UCampBuildingPanelWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;
	TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Title"));
	HeaderIcon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("HeaderIcon"));
	LevelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Level"));
	EffectText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Effect"));
	CostText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Cost"));
	UpgradeButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("UpgradeButton"));
	CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CloseButton"));
	UTextBlock* UpgradeText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("UpgradeText"));
	UpgradeText->SetText(NSLOCTEXT("Camp", "Upgrade", "Upgrade"));
	UpgradeButton->AddChild(UpgradeText);
	UpgradeButton->OnClicked.AddDynamic(this, &UCampBuildingPanelWidget::HandleUpgrade);
	UTextBlock* CloseText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CloseText"));
	CloseText->SetText(NSLOCTEXT("Camp", "Close", "Close"));
	CloseButton->AddChild(CloseText);
	CloseButton->OnClicked.AddDynamic(this, &UCampBuildingPanelWidget::HandleClose);
	Root->AddChildToVerticalBox(HeaderIcon);
	Root->AddChildToVerticalBox(TitleText);
	Root->AddChildToVerticalBox(LevelText);
	Root->AddChildToVerticalBox(EffectText);
	Root->AddChildToVerticalBox(CostText);
	Root->AddChildToVerticalBox(UpgradeButton);
	Root->AddChildToVerticalBox(CloseButton);
}

void UCampBuildingPanelWidget::SetBuilding(UBuildingData* InBuildingData, bool bInLocked, FText InLockReason)
{
	BuildingData = InBuildingData;
	bLocked = bInLocked;
	LockReason = InLockReason;
	Refresh();
}

void UCampBuildingPanelWidget::Refresh()
{
	USaveManagerSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<USaveManagerSubsystem>() : nullptr;
	if (!BuildingData || !Save)
	{
		return;
	}
	const int32 CurrentLevel = Save->GetBuildingLevel(BuildingData->Id);
	TitleText->SetText(BuildingData->DisplayName);
	if (HeaderIcon)
	{
		if (UTexture2D* Texture = EmberGeneratedArt::LoadTexture(EmberGeneratedArt::BuildingTexturePath(BuildingData->Id)))
		{
			HeaderIcon->SetBrushFromTexture(Texture, true);
		}
	}
	LevelText->SetText(FText::FromString(FString::Printf(TEXT("Level %d/%d"), CurrentLevel, BuildingData->MaxLevel)));
	if (bLocked)
	{
		EffectText->SetText(LockReason);
		CostText->SetText(FText::GetEmpty());
		UpgradeButton->SetIsEnabled(false);
		return;
	}
	const float Effect = BuildingData->PerLevelEffectMagnitude.IsValidIndex(FMath::Max(0, CurrentLevel - 1)) ? BuildingData->PerLevelEffectMagnitude[FMath::Max(0, CurrentLevel - 1)] : 0.0f;
	EffectText->SetText(FText::FromString(FString::Printf(TEXT("Effect: %.1f per level"), Effect)));
	bool bAffordable = false;
	CostText->SetText(FText::FromString(CostLinesForLevel(CurrentLevel <= 0 ? 1 : CurrentLevel, bAffordable)));
	UpgradeButton->SetIsEnabled(CurrentLevel < BuildingData->MaxLevel && bAffordable);
}

FString UCampBuildingPanelWidget::CostLinesForLevel(int32 CurrentLevel, bool& bAffordable) const
{
	bAffordable = false;
	UEconomySubsystem* Economy = GetGameInstance() ? GetGameInstance()->GetSubsystem<UEconomySubsystem>() : nullptr;
	if (!BuildingData || !Economy)
	{
		return TEXT("No economy");
	}
	if (CurrentLevel >= BuildingData->MaxLevel)
	{
		return TEXT("Max level");
	}
	bAffordable = USaveManagerSubsystem::CanAffordUpgrade(BuildingData, CurrentLevel, Economy);
	if (!BuildingData->UpgradeCosts.IsValidIndex(CurrentLevel - 1))
	{
		return TEXT("No cost data");
	}
	TArray<FString> Lines;
	for (const TPair<EEmberResource, int32>& Pair : BuildingData->UpgradeCosts[CurrentLevel - 1].Amounts)
	{
		Lines.Add(FString::Printf(TEXT("%s: %d/%d%s"), *ResourceLabel(Pair.Key), Economy->Get(Pair.Key), Pair.Value, Economy->Get(Pair.Key) >= Pair.Value ? TEXT("") : TEXT(" missing")));
	}
	return FString::Join(Lines, TEXT("\n"));
}

void UCampBuildingPanelWidget::HandleUpgrade()
{
	if (USaveManagerSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<USaveManagerSubsystem>() : nullptr)
	{
		Save->PurchaseBuilding(BuildingData);
	}
	Refresh();
}

void UCampBuildingPanelWidget::HandleClose()
{
	RemoveFromParent();
}

#include "UI/EmberHUDWidget.h"
#include "Combat/EmberPlayerCharacter.h"
#include "Combat/HealthComponent.h"
#include "Combat/XPComponent.h"
#include "Core/EconomySubsystem.h"
#include "Generated/EmberGeneratedArt.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/SafeZone.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "EmberboundIsekai.h"

void UEmberHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	USafeZone* SafeRoot = WidgetTree->ConstructWidget<USafeZone>(USafeZone::StaticClass(), TEXT("SafeRoot"));
	UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Root"));
	SafeRoot->AddChild(Root);
	WidgetTree->RootWidget = SafeRoot;

	HPBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HPBar"));
	XPBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("XPBar"));
	BossHPBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("BossHPBar"));
	BossText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("BossText"));
	LevelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LevelText"));
	RoomText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RoomText"));
	ResourceText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ResourceText"));
	UHorizontalBox* IconRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ResourceIconRow"));

	int32 ResolvedIcons = 0;
	const TArray<FName> IconIds = EmberGeneratedArt::HudIconIds();
	for (FName IconId : IconIds)
	{
		UImage* Icon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		if (UTexture2D* Texture = EmberGeneratedArt::LoadTexture(EmberGeneratedArt::HudIconTexturePath(IconId)))
		{
			Icon->SetBrushFromTexture(Texture, false);
			Icon->SetDesiredSizeOverride(FVector2D(28.0f, 28.0f));
			++ResolvedIcons;
		}
		IconRow->AddChildToHorizontalBox(Icon);
	}
	UE_LOG(LogEmberbound, Display, TEXT("EMBER: IconResolved %d/%d"), ResolvedIcons, IconIds.Num());

	Root->AddChildToVerticalBox(IconRow);
	Root->AddChildToVerticalBox(HPBar);
	Root->AddChildToVerticalBox(XPBar);
	Root->AddChildToVerticalBox(BossText);
	Root->AddChildToVerticalBox(BossHPBar);
	Root->AddChildToVerticalBox(LevelText);
	Root->AddChildToVerticalBox(RoomText);
	Root->AddChildToVerticalBox(ResourceText);
	BossText->SetVisibility(ESlateVisibility::Collapsed);
	BossHPBar->SetVisibility(ESlateVisibility::Collapsed);
}

void UEmberHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	AEmberPlayerCharacter* Player = Cast<AEmberPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (Player)
	{
		if (UHealthComponent* Health = Player->GetHealthComponent())
		{
			HPBar->SetPercent(Health->GetMaxHP() > 0.0f ? Health->GetCurrentHP() / Health->GetMaxHP() : 0.0f);
		}
		if (UXPComponent* XP = Player->GetXPComponent())
		{
			XPBar->SetPercent(XP->GetXPToNext() > 0 ? static_cast<float>(XP->GetXP()) / static_cast<float>(XP->GetXPToNext()) : 0.0f);
			LevelText->SetText(FText::Format(NSLOCTEXT("EmberHUD", "LevelFmt", "Level {0}"), XP->GetLevel()));
		}
	}
	RoomText->SetText(FText::Format(NSLOCTEXT("EmberHUD", "RoomFmt", "Room {0}/{1}"), CurrentRoom, TotalRooms));
	if (BossHealth.IsValid())
	{
		BossText->SetVisibility(ESlateVisibility::HitTestInvisible);
		BossHPBar->SetVisibility(ESlateVisibility::HitTestInvisible);
		BossText->SetText(NSLOCTEXT("EmberHUD", "BossName", "Tutorial Dragon Lv.3"));
		BossHPBar->SetPercent(BossHealth->GetMaxHP() > 0.0f ? BossHealth->GetCurrentHP() / BossHealth->GetMaxHP() : 0.0f);
	}
	else
	{
		BossText->SetVisibility(ESlateVisibility::Collapsed);
		BossHPBar->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UEconomySubsystem* Economy = GameInstance->GetSubsystem<UEconomySubsystem>())
		{
			ResourceText->SetText(FText::Format(NSLOCTEXT("EmberHUD", "ResourcesFmt", "Gold {0}  Wood {1}  Ash {2}"),
				Economy->Get(EEmberResource::Gold),
				Economy->Get(EEmberResource::Wood),
				Economy->Get(EEmberResource::SoulAsh)));
		}
	}
}

void UEmberHUDWidget::SetRoom(int32 Room, int32 MaxRooms)
{
	CurrentRoom = Room;
	TotalRooms = MaxRooms;
}

void UEmberHUDWidget::SetBossHealthSource(UHealthComponent* InBossHealth)
{
	BossHealth = InBossHealth;
}

void UEmberHUDWidget::ClearBossHealthSource()
{
	BossHealth.Reset();
}

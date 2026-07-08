#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Data/MonsterData.h"
#include "CampBuildingPanelWidget.generated.h"

class UBuildingData;
class UButton;
class UImage;
class UTextBlock;
class UVerticalBox;

UCLASS()
class EMBERBOUNDISEKAI_API UMonsterAssignButton : public UButton
{
	GENERATED_BODY()

public:
	FName MonsterId;
	EMonsterAssignmentRole Role = EMonsterAssignmentRole::None;

	UFUNCTION()
	void HandleClicked();
};

UCLASS()
class EMBERBOUNDISEKAI_API UCampBuildingPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	void SetBuilding(UBuildingData* InBuildingData, bool bInLocked, FText InLockReason);

private:
	UPROPERTY()
	TObjectPtr<UBuildingData> BuildingData;

	UPROPERTY()
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY()
	TObjectPtr<UImage> HeaderIcon;

	UPROPERTY()
	TObjectPtr<UTextBlock> LevelText;

	UPROPERTY()
	TObjectPtr<UTextBlock> EffectText;

	UPROPERTY()
	TObjectPtr<UTextBlock> CostText;

	UPROPERTY()
	TObjectPtr<UButton> UpgradeButton;

	UPROPERTY()
	TObjectPtr<UButton> CloseButton;

	UPROPERTY()
	TObjectPtr<UVerticalBox> DetailBox;

	bool bLocked = false;
	FText LockReason;

	void Refresh();
	void RefreshMonsterPanel();
	void RefreshWatchtowerPanel();
	FString CostLinesForLevel(int32 CurrentLevel, bool& bAffordable) const;

	UFUNCTION()
	void HandleUpgrade();

	UFUNCTION()
	void HandleClose();
};

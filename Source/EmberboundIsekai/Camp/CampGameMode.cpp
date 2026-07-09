#include "Camp/CampGameMode.h"
#include "Camp/CampBuildingActor.h"
#include "Combat/EmberPlayerCharacter.h"
#include "Core/EmberAutopilotSubsystem.h"
#include "Core/EmberGameInstance.h"
#include "Core/SaveManagerSubsystem.h"
#include "Core/MonsterCollectionSubsystem.h"
#include "Core/EmberCaptureDebug.h"
#include "UI/CampHUDWidget.h"
#include "UI/OfflineRewardWidget.h"
#include "UI/OnboardingHintWidget.h"
#include "UI/VirtualJoystickWidget.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"
#include "EmberboundIsekai.h"

namespace
{
	void SpawnCampFloor(UWorld* World, const TCHAR* Label, const FVector& Location, const FVector& Scale, const TCHAR* MaterialPath)
	{
		if (!World)
		{
			return;
		}
		UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
		UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, MaterialPath);
		if (!PlaneMesh || !Material)
		{
			return;
		}
		AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
		if (Floor && Floor->GetStaticMeshComponent())
		{
#if WITH_EDITOR
			Floor->SetActorLabel(Label);
#endif
			Floor->GetStaticMeshComponent()->SetStaticMesh(PlaneMesh);
			Floor->GetStaticMeshComponent()->SetMaterial(0, Material);
			Floor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Floor->SetActorScale3D(Scale);
		}
	}
}

ACampGameMode::ACampGameMode()
{
	DefaultPawnClass = AEmberPlayerCharacter::StaticClass();
}

void ACampGameMode::BeginPlay()
{
	Super::BeginPlay();
	SpawnCampFloor(GetWorld(), TEXT("M15_CampMeadowFloor_Fullscreen2D"), FVector(0.0f, 0.0f, 6.0f), FVector(120.0f, 120.0f, 1.0f), TEXT("/Game/Emberbound/Materials/Generated/MI_CampFloor_MeadowGrass.MI_CampFloor_MeadowGrass"));
	if (USaveManagerSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<USaveManagerSubsystem>() : nullptr)
	{
		const bool bBankedRun = Save->BankPendingRunResult();
		if (!bBankedRun)
		{
			if (UMonsterCollectionSubsystem* Monsters = GetGameInstance()->GetSubsystem<UMonsterCollectionSubsystem>())
			{
				const FOfflineRewardResult Reward = Monsters->ApplyOfflineIncome();
				if (Reward.Minutes > 0 && Reward.Rewards.Num() > 0)
				{
					if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
					{
						UOfflineRewardWidget* RewardWidget = CreateWidget<UOfflineRewardWidget>(PC, UOfflineRewardWidget::StaticClass());
						if (RewardWidget)
						{
							RewardWidget->SetReward(Reward);
							RewardWidget->AddToViewport(160);
						}
					}
				}
			}
			Save->SaveNow();
		}
	}
	if (AEmberPlayerCharacter* Player = Cast<AEmberPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		Player->SetAutoAttackEnabled(false);
		Player->SetCameraOrthoWidth(980.0f);
		Player->SetActorLocation(FVector(-150.0f, 0.0f, Player->GetActorLocation().Z));
	}
	const UEmberGameInstance* EmberGI = GetGameInstance() ? Cast<UEmberGameInstance>(GetGameInstance()) : nullptr;
	const bool bMainMenuActive = EmberGI && (EmberGI->IsMainMenuActive() || EmberGI->WillShowInitialMainMenu());
	if (!bMainMenuActive)
	{
		StartCampAfterMenu();
	}
	FTimerHandle JoystickCheckHandle;
	if (!bMainMenuActive)
	{
		GetWorldTimerManager().SetTimer(JoystickCheckHandle, this, &ACampGameMode::ShowJoystickIfNeeded, 0.5f, true);
	}
	FEmberCaptureDebug::ScheduleFromCommandLine(GetWorld());
	if (UEmberAutopilotSubsystem* Autopilot = GetGameInstance() ? GetGameInstance()->GetSubsystem<UEmberAutopilotSubsystem>() : nullptr)
	{
		Autopilot->OnCampBeginPlay(this);
	}
}

void ACampGameMode::StartCampAfterMenu()
{
	if (bCampPresentationStarted)
	{
		return;
	}
	bCampPresentationStarted = true;
	SpawnCampActors();
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		PC->bEnableClickEvents = true;
		PC->bEnableTouchEvents = true;
		CampHUD = CreateWidget<UCampHUDWidget>(PC, UCampHUDWidget::StaticClass());
		if (CampHUD)
		{
			CampHUD->AddToViewport();
		}
	}
	UOnboardingHintWidget::ShowCards(this, false);
	ShowJoystickIfNeeded();
}

void ACampGameMode::SpawnCampActors()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	for (const FCampBuildingLayoutSpec& Spec : ACampBuildingActor::CampLayoutSpecs())
	{
		if (Spec.BuildingId == TEXT("campfire-core"))
		{
			World->SpawnActor<ACampfireCoreActor>(Spec.Location, FRotator::ZeroRotator);
		}
		else if (Spec.BuildingId == TEXT("grim-forge"))
		{
			World->SpawnActor<AGrimForgeActor>(Spec.Location, FRotator::ZeroRotator);
		}
		else if (Spec.BuildingId == TEXT("monster-pen"))
		{
			World->SpawnActor<AMonsterPenActor>(Spec.Location, FRotator::ZeroRotator);
		}
		else if (Spec.BuildingId == TEXT("watchtower"))
		{
			World->SpawnActor<AWatchtowerActor>(Spec.Location, FRotator::ZeroRotator);
		}
		else if (Spec.BuildingId == TEXT("expedition-portal"))
		{
			World->SpawnActor<AExpeditionPortalActor>(Spec.Location, FRotator::ZeroRotator);
		}
	}
}

void ACampGameMode::ShowJoystickIfNeeded()
{
	if (JoystickWidget || !UVirtualJoystickWidget::ShouldShowJoystick(this))
	{
		return;
	}
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		JoystickWidget = CreateWidget<UVirtualJoystickWidget>(PC, UVirtualJoystickWidget::StaticClass());
		if (JoystickWidget)
		{
			JoystickWidget->AddToViewport(80);
			UE_LOG(LogEmberbound, Log, TEXT("EMBER: JoystickShown"));
		}
	}
}

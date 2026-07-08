#include "Camp/CampGameMode.h"
#include "Camp/CampBuildingActor.h"
#include "Combat/EmberPlayerCharacter.h"
#include "Core/SaveManagerSubsystem.h"
#include "UI/CampHUDWidget.h"
#include "Kismet/GameplayStatics.h"

ACampGameMode::ACampGameMode()
{
	DefaultPawnClass = AEmberPlayerCharacter::StaticClass();
}

void ACampGameMode::BeginPlay()
{
	Super::BeginPlay();
	if (USaveManagerSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<USaveManagerSubsystem>() : nullptr)
	{
		Save->BankPendingRunResult();
		Save->SaveNow();
	}
	SpawnCampActors();
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		CampHUD = CreateWidget<UCampHUDWidget>(PC, UCampHUDWidget::StaticClass());
		if (CampHUD)
		{
			CampHUD->AddToViewport();
		}
	}
}

void ACampGameMode::SpawnCampActors()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	World->SpawnActor<ACampfireCoreActor>(FVector(0.0f, 0.0f, 70.0f), FRotator::ZeroRotator);
	World->SpawnActor<AGrimForgeActor>(FVector(360.0f, 0.0f, 70.0f), FRotator::ZeroRotator);
	World->SpawnActor<AMonsterPenActor>(FVector(-360.0f, 0.0f, 70.0f), FRotator::ZeroRotator);
	World->SpawnActor<AWatchtowerActor>(FVector(0.0f, 360.0f, 70.0f), FRotator::ZeroRotator);
	World->SpawnActor<AExpeditionPortalActor>(FVector(0.0f, -420.0f, 70.0f), FRotator::ZeroRotator);
}

#include "Camp/CampGameMode.h"
#include "Camp/CampBuildingActor.h"
#include "Combat/EmberPlayerCharacter.h"
#include "Core/SaveManagerSubsystem.h"
#include "UI/CampHUDWidget.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"

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
			Floor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
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
	SpawnCampFloor(GetWorld(), TEXT("M5b_CampMeadowFloor"), FVector(0.0f, 0.0f, -2.0f), FVector(24.0f, 24.0f, 1.0f), TEXT("/Game/Emberbound/Materials/Generated/MI_CampFloor_MeadowGrass.MI_CampFloor_MeadowGrass"));
	SpawnCampFloor(GetWorld(), TEXT("M5b_CampDirtPatch"), FVector(0.0f, 0.0f, -1.0f), FVector(7.0f, 7.0f, 1.0f), TEXT("/Game/Emberbound/Materials/Generated/MI_CampDirt.MI_CampDirt"));
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

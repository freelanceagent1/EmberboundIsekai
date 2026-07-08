#include "Camp/CampBuildingActor.h"
#include "Core/SaveManagerSubsystem.h"
#include "Core/SceneFlowSubsystem.h"
#include "Data/BuildingData.h"
#include "Generated/EmberGeneratedArt.h"
#include "UI/CampBuildingPanelWidget.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ACampBuildingActor::ACampBuildingActor()
{
	PrimaryActorTick.bCanEverTick = true;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;
	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(SceneRoot);
	VisualMesh->SetHiddenInGame(true);
	VisualMesh->SetVisibility(false);

	BillboardVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BillboardVisual"));
	BillboardVisual->SetupAttachment(SceneRoot);
	BillboardVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FlameVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlameVisual"));
	FlameVisual->SetupAttachment(SceneRoot);
	FlameVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlameVisual->SetHiddenInGame(true);
	FlameVisual->SetVisibility(false);

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->InitSphereRadius(180.0f);
	InteractionSphere->SetupAttachment(SceneRoot);
	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACampBuildingActor::HandleBeginOverlap);
	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh)
	{
		VisualMesh->SetStaticMesh(CubeMesh);
	}
	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh)
	{
		BillboardVisual->SetStaticMesh(PlaneMesh);
		BillboardVisual->SetRelativeLocation(FVector(0.0f, 0.0f, 40.0f));
		BillboardVisual->SetRelativeScale3D(FVector(1.8f, 1.0f, 1.35f));
		FlameVisual->SetStaticMesh(PlaneMesh);
		FlameVisual->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
		FlameVisual->SetRelativeScale3D(FVector(0.7f, 1.0f, 0.7f));
	}
}

void ACampBuildingActor::BeginPlay()
{
	Super::BeginPlay();
	BuildingData = LoadBuildingData();
	ConfigureVisual();
}

void ACampBuildingActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	EmberGeneratedArt::FaceComponentToPlayerCamera(BillboardVisual, this);
	EmberGeneratedArt::FaceComponentToPlayerCamera(FlameVisual, this);
}

bool ACampBuildingActor::IsInteractionLocked(FText& OutReason) const
{
	return false;
}

void ACampBuildingActor::ConfigureVisual()
{
	VisualMesh->SetWorldScale3D(FVector(1.2f, 1.2f, 1.0f));
	if (!BuildingId.IsNone())
	{
		ConfigureBillboardMaterial(BillboardVisual, EmberGeneratedArt::BuildingTexturePath(BuildingId));
	}
}

void ACampBuildingActor::ConfigureBillboardMaterial(UStaticMeshComponent* Component, const FString& TexturePath, UMaterialInterface* OverrideMaterial)
{
	if (!Component)
	{
		return;
	}
	UMaterialInterface* Material = OverrideMaterial ? OverrideMaterial : EmberGeneratedArt::LoadBillboardMaterial();
	if (!Material)
	{
		return;
	}
	Component->SetMaterial(0, Material);
	if (UMaterialInstanceDynamic* MID = Component->CreateAndSetMaterialInstanceDynamic(0))
	{
		MID->SetTextureParameterValue(TEXT("Tex"), EmberGeneratedArt::LoadTexture(TexturePath));
		MID->SetVectorParameterValue(TEXT("Tint"), FLinearColor::White);
		MID->SetScalarParameterValue(TEXT("Flash"), 0.0f);
	}
}

void ACampBuildingActor::OpenPanel()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC || !BuildingData)
	{
		return;
	}
	UCampBuildingPanelWidget* Panel = CreateWidget<UCampBuildingPanelWidget>(PC, UCampBuildingPanelWidget::StaticClass());
	if (Panel)
	{
		FText LockReason;
		Panel->SetBuilding(BuildingData, IsInteractionLocked(LockReason), LockReason);
		Panel->AddToViewport(100);
		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeGameAndUI());
	}
}

UBuildingData* ACampBuildingActor::LoadBuildingData() const
{
	if (BuildingId.IsNone())
	{
		return nullptr;
	}
	if (BuildingId == TEXT("expedition-portal"))
	{
		return nullptr;
	}
	const TMap<FName, FString> AssetNames = {
		{ TEXT("campfire-core"), TEXT("DA_CampfireCore") },
		{ TEXT("grim-forge"), TEXT("DA_GrimForge") },
		{ TEXT("monster-pen"), TEXT("DA_MonsterPen") },
		{ TEXT("watchtower"), TEXT("DA_Watchtower") }
	};
	const FString AssetName = AssetNames.Contains(BuildingId) ? AssetNames[BuildingId] : BuildingId.ToString();
	const FString Path = FString::Printf(TEXT("/Game/Emberbound/Data/Buildings/%s.%s"), *AssetName, *AssetName);
	return LoadObject<UBuildingData>(nullptr, *Path);
}

void ACampBuildingActor::HandleBeginOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (OtherActor == UGameplayStatics::GetPlayerPawn(this, 0))
	{
		OpenPanel();
	}
}

ACampfireCoreActor::ACampfireCoreActor()
{
	BuildingId = TEXT("campfire-core");
}

void ACampfireCoreActor::ConfigureVisual()
{
	VisualMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 0.7f));
	ConfigureBillboardMaterial(BillboardVisual, EmberGeneratedArt::BuildingTexturePath(BuildingId));
	if (FlameVisual)
	{
		FlameVisual->SetHiddenInGame(false);
		FlameVisual->SetVisibility(true);
		ConfigureBillboardMaterial(FlameVisual, TEXT("/Game/Emberbound/Generated/VFX/T_CampfireFlame.T_CampfireFlame"), EmberGeneratedArt::LoadCampfireFlameMaterial());
	}
	if (USaveManagerSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<USaveManagerSubsystem>() : nullptr)
	{
		const float RadiusScale = 1.0f + 0.35f * static_cast<float>(Save->GetBuildingLevel(BuildingId));
		InteractionSphere->SetSphereRadius(180.0f * RadiusScale);
	}
}

AGrimForgeActor::AGrimForgeActor()
{
	BuildingId = TEXT("grim-forge");
}

AMonsterPenActor::AMonsterPenActor()
{
	BuildingId = TEXT("monster-pen");
}

bool AMonsterPenActor::IsInteractionLocked(FText& OutReason) const
{
	const USaveManagerSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<USaveManagerSubsystem>() : nullptr;
	if (!Save || Save->GetBuildingLevel(TEXT("campfire-core")) < 2)
	{
		OutReason = NSLOCTEXT("Camp", "MonsterPenLocked", "Requires Campfire Core level 2");
		return true;
	}
	return false;
}

AWatchtowerActor::AWatchtowerActor()
{
	BuildingId = TEXT("watchtower");
}

AExpeditionPortalActor::AExpeditionPortalActor()
{
	BuildingId = TEXT("expedition-portal");
}

void AExpeditionPortalActor::OpenPanel()
{
	if (USceneFlowSubsystem* Flow = GetGameInstance() ? GetGameInstance()->GetSubsystem<USceneFlowSubsystem>() : nullptr)
	{
		Flow->TravelToExpedition();
	}
}

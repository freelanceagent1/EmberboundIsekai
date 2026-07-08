#include "Camp/CampBuildingActor.h"
#include "Core/SaveManagerSubsystem.h"
#include "Core/SceneFlowSubsystem.h"
#include "Data/BuildingData.h"
#include "UI/CampBuildingPanelWidget.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ACampBuildingActor::ACampBuildingActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;
	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(SceneRoot);
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->InitSphereRadius(180.0f);
	InteractionSphere->SetupAttachment(SceneRoot);
	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACampBuildingActor::HandleBeginOverlap);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		VisualMesh->SetStaticMesh(CubeMesh.Object);
	}
}

void ACampBuildingActor::BeginPlay()
{
	Super::BeginPlay();
	BuildingData = LoadBuildingData();
	ConfigureVisual();
}

bool ACampBuildingActor::IsInteractionLocked(FText& OutReason) const
{
	return false;
}

void ACampBuildingActor::ConfigureVisual()
{
	VisualMesh->SetWorldScale3D(FVector(1.2f, 1.2f, 1.0f));
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
	BuildingId = NAME_None;
}

void AExpeditionPortalActor::OpenPanel()
{
	if (USceneFlowSubsystem* Flow = GetGameInstance() ? GetGameInstance()->GetSubsystem<USceneFlowSubsystem>() : nullptr)
	{
		Flow->TravelToExpedition();
	}
}

#include "Expedition/ExpeditionPortal.h"
#include "Combat/EmberPlayerCharacter.h"
#include "Generated/EmberGeneratedArt.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

AExpeditionPortal::AExpeditionPortal()
{
	PrimaryActorTick.bCanEverTick = true;
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitSphereRadius(90.0f);
	Collision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AExpeditionPortal::HandleOverlap);

	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(RootComponent);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh)
	{
		Visual->SetStaticMesh(PlaneMesh);
		Visual->SetRelativeLocation(FVector(0.0f, 0.0f, 90.0f));
		Visual->SetRelativeScale3D(FVector(2.3f, 2.3f, 1.0f));
	}
	if (UMaterialInterface* Material = EmberGeneratedArt::LoadBillboardMaterial())
	{
		Visual->SetMaterial(0, Material);
		PortalMID = Visual->CreateAndSetMaterialInstanceDynamic(0);
		if (PortalMID)
		{
			PortalMID->SetTextureParameterValue(TEXT("Tex"), EmberGeneratedArt::LoadTexture(EmberGeneratedArt::BuildingTexturePath(TEXT("expedition-portal"))));
			PortalMID->SetVectorParameterValue(TEXT("Tint"), FLinearColor(1.0f, 0.44f, 0.08f, 1.0f));
			PortalMID->SetScalarParameterValue(TEXT("Flash"), 0.0f);
		}
	}
}

void AExpeditionPortal::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	EmberGeneratedArt::FaceComponentToPlayerCamera(Visual, this);
	if (PortalMID && GetWorld())
	{
		const float Pulse = 0.5f + 0.5f * FMath::Sin(GetWorld()->GetTimeSeconds() * 5.0f);
		PortalMID->SetScalarParameterValue(TEXT("Flash"), Pulse);
	}
}

void AExpeditionPortal::HandleOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (!bUsed && OtherActor && OtherActor->IsA<AEmberPlayerCharacter>())
	{
		bUsed = true;
		OnPortalEntered.ExecuteIfBound();
	}
}

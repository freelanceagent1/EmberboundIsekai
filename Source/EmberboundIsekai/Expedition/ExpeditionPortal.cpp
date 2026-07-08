#include "Expedition/ExpeditionPortal.h"
#include "Combat/EmberPlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AExpeditionPortal::AExpeditionPortal()
{
	PrimaryActorTick.bCanEverTick = false;
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitSphereRadius(90.0f);
	Collision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AExpeditionPortal::HandleOverlap);

	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(RootComponent);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UStaticMesh* CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh)
	{
		Visual->SetStaticMesh(CylinderMesh);
		Visual->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.08f));
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

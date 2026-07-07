#include "Enemies/BossTelegraphActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

ABossTelegraphActor::ABossTelegraphActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	SetRootComponent(Visual);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Visual->SetStaticMesh(CubeMesh.Object);
	}
}

void ABossTelegraphActor::Configure(const FVector& Center, const FRotator& Rotation, float Length, float Width, float Duration)
{
	SetActorLocation(Center);
	SetActorRotation(Rotation);
	Visual->SetRelativeScale3D(FVector(FMath::Max(1.0f, Length) / 100.0f, FMath::Max(1.0f, Width) / 100.0f, 0.03f));
	if (UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Emberbound/Materials/Stylized/M_Projectile.M_Projectile")))
	{
		Visual->SetMaterial(0, Material);
		if (UMaterialInstanceDynamic* MID = Visual->CreateAndSetMaterialInstanceDynamic(0))
		{
			MID->SetVectorParameterValue(TEXT("Tint"), FLinearColor(1.0f, 0.25f, 0.05f, 0.8f));
			MID->SetScalarParameterValue(TEXT("Flash"), 1.0f);
		}
	}
	SetLifeSpan(FMath::Max(0.1f, Duration));
}

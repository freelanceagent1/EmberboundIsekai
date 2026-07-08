#include "Run/EmberSoulFlamePatch.h"
#include "Combat/HealthComponent.h"
#include "Enemies/EmberEnemyBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

AEmberSoulFlamePatch::AEmberSoulFlamePatch()
{
	PrimaryActorTick.bCanEverTick = true;
	SetLifeSpan(2.0f);
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitSphereRadius(70.0f);
	Collision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AEmberSoulFlamePatch::HandleBeginOverlap);
	Collision->OnComponentEndOverlap.AddDynamic(this, &AEmberSoulFlamePatch::HandleEndOverlap);
	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(RootComponent);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UStaticMesh* CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh)
	{
		Visual->SetStaticMesh(CylinderMesh);
		Visual->SetRelativeScale3D(FVector(0.65f, 0.65f, 0.03f));
	}
}

void AEmberSoulFlamePatch::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	DamageAccumulator += DeltaSeconds * 5.0f;
	if (DamageAccumulator < 1.0f)
	{
		return;
	}
	const float Damage = FMath::FloorToFloat(DamageAccumulator);
	DamageAccumulator -= Damage;
	for (TWeakObjectPtr<AActor> Enemy : TouchingEnemies)
	{
		if (AActor* Actor = Enemy.Get())
		{
			if (UHealthComponent* Health = Actor->FindComponentByClass<UHealthComponent>())
			{
				Health->ApplyDamage(Damage);
			}
		}
	}
}

void AEmberSoulFlamePatch::HandleBeginOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (OtherActor && OtherActor->IsA<AEmberEnemyBase>())
	{
		TouchingEnemies.Add(OtherActor);
	}
}

void AEmberSoulFlamePatch::HandleEndOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32)
{
	TouchingEnemies.Remove(OtherActor);
}

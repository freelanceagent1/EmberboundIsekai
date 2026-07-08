#include "Run/EmberBoneShieldOrbiter.h"
#include "Combat/EmberPlayerCharacter.h"
#include "Combat/HealthComponent.h"
#include "Enemies/EmberEnemyBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

AEmberBoneShieldOrbiter::AEmberBoneShieldOrbiter()
{
	PrimaryActorTick.bCanEverTick = true;
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitSphereRadius(32.0f);
	Collision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AEmberBoneShieldOrbiter::HandleOverlap);
	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(RootComponent);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh)
	{
		Visual->SetStaticMesh(SphereMesh);
		Visual->SetRelativeScale3D(FVector(0.22f));
	}
}

void AEmberBoneShieldOrbiter::AttachToPlayer(AEmberPlayerCharacter* InPlayer, int32 InIndex)
{
	Player = InPlayer;
	OrbitIndex = InIndex;
	OrbitAngle = InIndex * 120.0f;
}

void AEmberBoneShieldOrbiter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!Player)
	{
		return;
	}
	OrbitAngle += DeltaSeconds * 180.0f;
	const float Radians = FMath::DegreesToRadians(OrbitAngle + OrbitIndex * 120.0f);
	const FVector Offset(FMath::Cos(Radians) * 135.0f, FMath::Sin(Radians) * 135.0f, 45.0f);
	SetActorLocation(Player->GetActorLocation() + Offset);
}

void AEmberBoneShieldOrbiter::HandleOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (!OtherActor || !OtherActor->IsA<AEmberEnemyBase>())
	{
		return;
	}
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	if (const float* Next = HitCooldowns.Find(OtherActor); Next && *Next > Now)
	{
		return;
	}
	HitCooldowns.Add(OtherActor, Now + 0.35f);
	if (UHealthComponent* Health = OtherActor->FindComponentByClass<UHealthComponent>())
	{
		Health->ApplyDamage(8.0f);
	}
}

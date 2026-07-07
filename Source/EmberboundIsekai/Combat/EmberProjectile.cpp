#include "Combat/EmberProjectile.h"
#include "Combat/HealthComponent.h"
#include "Combat/EmberPlayerCharacter.h"
#include "Core/ObjectPoolSubsystem.h"
#include "Enemies/EmberEnemyBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "TimerManager.h"

AEmberProjectile::AEmberProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitSphereRadius(18.0f);
	Collision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AEmberProjectile::HandleOverlap);

	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(RootComponent);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Visual->SetStaticMesh(SphereMesh.Object);
		Visual->SetRelativeScale3D(FVector(0.16f));
	}
}

void AEmberProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	AddActorWorldOffset(MoveDirection * Speed * DeltaSeconds, true);
}

void AEmberProjectile::OnAcquired_Implementation()
{
	SetLifeSpan(0.0f);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(LifetimeTimerHandle, this, &AEmberProjectile::ReleaseSelf, 2.0f, false);
	}
}

void AEmberProjectile::OnReleased_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LifetimeTimerHandle);
	}
	SourceActor = nullptr;
	Damage = 0.0f;
}

void AEmberProjectile::Fire(const FVector& Direction, float InDamage, float InSpeed, int32 InPierceCount, AActor* InSource, bool bInEnemyProjectile)
{
	MoveDirection = Direction.GetSafeNormal2D();
	if (MoveDirection.IsNearlyZero())
	{
		MoveDirection = GetActorForwardVector();
	}
	Damage = InDamage;
	Speed = InSpeed;
	PierceRemaining = InPierceCount;
	SourceActor = InSource;
	bEnemyProjectile = bInEnemyProjectile;
	SetActorRotation(MoveDirection.Rotation());
}

void AEmberProjectile::HandleOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (!OtherActor || OtherActor == this || OtherActor == SourceActor)
	{
		return;
	}
	const bool bValidTarget = bEnemyProjectile ? OtherActor->IsA<AEmberPlayerCharacter>() : OtherActor->IsA<AEmberEnemyBase>();
	if (!bValidTarget)
	{
		return;
	}
	if (UHealthComponent* Health = OtherActor->FindComponentByClass<UHealthComponent>())
	{
		Health->ApplyDamage(Damage);
		if (PierceRemaining > 0)
		{
			--PierceRemaining;
		}
		else
		{
			ReleaseSelf();
		}
	}
}

void AEmberProjectile::ReleaseSelf()
{
	if (UWorld* World = GetWorld())
	{
		if (UObjectPoolSubsystem* Pool = World->GetSubsystem<UObjectPoolSubsystem>())
		{
			Pool->ReleaseActor(this);
			return;
		}
	}
	Destroy();
}

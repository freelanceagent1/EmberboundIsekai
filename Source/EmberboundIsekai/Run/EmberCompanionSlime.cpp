#include "Run/EmberCompanionSlime.h"
#include "Combat/EmberPlayerCharacter.h"
#include "Combat/EmberProjectile.h"
#include "Core/ObjectPoolSubsystem.h"
#include "Enemies/EmberEnemyBase.h"
#include "Enemies/EnemySpawnerSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "TimerManager.h"

AEmberCompanionSlime::AEmberCompanionSlime()
{
	PrimaryActorTick.bCanEverTick = true;
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitSphereRadius(34.0f);
	Collision->SetCollisionProfileName(TEXT("Pawn"));
	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(RootComponent);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Visual->SetStaticMesh(SphereMesh.Object);
		Visual->SetRelativeScale3D(FVector(0.45f, 0.45f, 0.3f));
	}
}

void AEmberCompanionSlime::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!Player)
	{
		return;
	}
	const FVector Desired = Player->GetActorLocation() + FVector(-140.0f, 95.0f, 0.0f);
	SetActorLocation(FMath::VInterpTo(GetActorLocation(), Desired, DeltaSeconds, 4.0f));
	if (!GetWorldTimerManager().IsTimerActive(FireTimerHandle))
	{
		GetWorldTimerManager().SetTimer(FireTimerHandle, this, &AEmberCompanionSlime::FireAcidBlob, 1.2f, true, 0.2f);
	}
}

void AEmberCompanionSlime::FireAcidBlob()
{
	if (!GetWorld())
	{
		return;
	}
	UEnemySpawnerSubsystem* Spawner = GetWorld()->GetSubsystem<UEnemySpawnerSubsystem>();
	UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
	if (!Spawner || !Pool)
	{
		return;
	}
	AEmberEnemyBase* Target = nullptr;
	float BestDistanceSq = FMath::Square(800.0f);
	for (AEmberEnemyBase* Enemy : Spawner->GetLiveEnemies())
	{
		if (!Enemy || !Enemy->IsAlive())
		{
			continue;
		}
		const float DistSq = FVector::DistSquared2D(GetActorLocation(), Enemy->GetActorLocation());
		if (DistSq < BestDistanceSq)
		{
			BestDistanceSq = DistSq;
			Target = Enemy;
		}
	}
	if (!Target)
	{
		return;
	}
	const FVector Direction = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	if (AEmberProjectile* Projectile = Pool->Acquire<AEmberProjectile>(AEmberProjectile::StaticClass(), FTransform(Direction.Rotation(), GetActorLocation() + FVector(0.0f, 0.0f, 35.0f))))
	{
		Projectile->Fire(Direction, 6.0f, 1000.0f, 0, 0, this, false);
	}
}

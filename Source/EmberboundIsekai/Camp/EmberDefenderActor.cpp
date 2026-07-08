#include "Camp/EmberDefenderActor.h"
#include "Combat/EmberProjectile.h"
#include "Core/ObjectPoolSubsystem.h"
#include "Data/MonsterData.h"
#include "Enemies/EmberEnemyBase.h"
#include "Enemies/EnemySpawnerSubsystem.h"
#include "Generated/EmberGeneratedArt.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

AEmberDefenderActor::AEmberDefenderActor()
{
	PrimaryActorTick.bCanEverTick = true;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;
	BillboardVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BillboardVisual"));
	BillboardVisual->SetupAttachment(SceneRoot);
	BillboardVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane")))
	{
		BillboardVisual->SetStaticMesh(PlaneMesh);
		BillboardVisual->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));
		BillboardVisual->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	}
}

void AEmberDefenderActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	EmberGeneratedArt::FaceComponentToPlayerCamera(BillboardVisual, this);
}

void AEmberDefenderActor::OnAcquired_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(FireTimerHandle, this, &AEmberDefenderActor::FireAtNearestEnemy, 1.0f, true, 0.25f);
	}
}

void AEmberDefenderActor::OnReleased_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FireTimerHandle);
	}
	MonsterData = nullptr;
}

void AEmberDefenderActor::InitDefender(UMonsterData* InMonsterData)
{
	MonsterData = InMonsterData;
	ConfigureVisual();
}

void AEmberDefenderActor::ConfigureVisual()
{
	if (!BillboardVisual || !MonsterData)
	{
		return;
	}
	if (UMaterialInterface* Material = EmberGeneratedArt::LoadBillboardMaterial())
	{
		BillboardVisual->SetMaterial(0, Material);
		if (UMaterialInstanceDynamic* MID = BillboardVisual->CreateAndSetMaterialInstanceDynamic(0))
		{
			MID->SetTextureParameterValue(TEXT("Tex"), MonsterData->BillboardTexture.LoadSynchronous());
			MID->SetVectorParameterValue(TEXT("Tint"), FLinearColor::White);
			MID->SetScalarParameterValue(TEXT("Flash"), 0.0f);
		}
	}
}

void AEmberDefenderActor::FireAtNearestEnemy()
{
	if (!MonsterData || MonsterData->DefenderDps <= 0.0f || !GetWorld())
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
	float BestDistanceSq = FMath::Square(1200.0f);
	for (AEmberEnemyBase* Enemy : Spawner->GetLiveEnemies())
	{
		if (!Enemy || !Enemy->IsAlive())
		{
			continue;
		}
		const float DistanceSq = FVector::DistSquared2D(GetActorLocation(), Enemy->GetActorLocation());
		if (DistanceSq <= BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			Target = Enemy;
		}
	}
	if (!Target)
	{
		return;
	}
	const FVector Direction = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	const FVector SpawnLocation = GetActorLocation() + Direction * 55.0f + FVector(0.0f, 0.0f, 70.0f);
	if (AEmberProjectile* Projectile = Pool->Acquire<AEmberProjectile>(AEmberProjectile::StaticClass(), FTransform(Direction.Rotation(), SpawnLocation)))
	{
		Projectile->Fire(Direction, MonsterData->DefenderDps, 1400.0f, 0, 0, this, false);
	}
}

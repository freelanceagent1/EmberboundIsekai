#include "Run/EmberCompanionActor.h"
#include "Combat/EmberPlayerCharacter.h"
#include "Combat/EmberProjectile.h"
#include "Core/ObjectPoolSubsystem.h"
#include "Data/MonsterData.h"
#include "Enemies/EmberEnemyBase.h"
#include "Enemies/EnemySpawnerSubsystem.h"
#include "Generated/EmberGeneratedArt.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"
#include "EmberboundIsekai.h"

AEmberCompanionActor::AEmberCompanionActor()
{
	PrimaryActorTick.bCanEverTick = true;
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitSphereRadius(34.0f);
	Collision->SetCollisionProfileName(TEXT("Pawn"));
	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(RootComponent);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane")))
	{
		Visual->SetStaticMesh(PlaneMesh);
		Visual->SetRelativeLocation(FVector(0.0f, 0.0f, 45.0f));
		Visual->SetRelativeScale3D(FVector(0.9f, 1.0f, 0.9f));
	}
}

void AEmberCompanionActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!Player)
	{
		return;
	}
	EmberGeneratedArt::FaceComponentToPlayerCamera(Visual, this);
	const FVector Desired = Player->GetActorLocation() + FVector(-140.0f, 95.0f, 0.0f);
	SetActorLocation(FMath::VInterpTo(GetActorLocation(), Desired, DeltaSeconds, 4.0f));
	if (MonsterData && !GetWorldTimerManager().IsTimerActive(FireTimerHandle))
	{
		GetWorldTimerManager().SetTimer(FireTimerHandle, this, &AEmberCompanionActor::FireProjectile, MonsterData->CompanionAttackInterval, true, 0.25f);
	}
}

void AEmberCompanionActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FireTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void AEmberCompanionActor::InitCompanion(UMonsterData* InMonsterData, AEmberPlayerCharacter* InPlayer)
{
	MonsterData = InMonsterData;
	Player = InPlayer;
	ConfigureVisual();
	if (MonsterData)
	{
		UE_LOG(LogEmberbound, Log, TEXT("EMBER: CompanionSpawned %s"), *MonsterData->Id.ToString());
	}
}

void AEmberCompanionActor::ConfigureVisual()
{
	if (!MonsterData || !Visual)
	{
		return;
	}
	if (UMaterialInterface* Material = EmberGeneratedArt::LoadBillboardMaterial())
	{
		Visual->SetMaterial(0, Material);
		VisualMID = Visual->CreateAndSetMaterialInstanceDynamic(0);
		if (VisualMID)
		{
			UTexture2D* Texture = MonsterData->BillboardTexture.LoadSynchronous();
			if (!Texture)
			{
				Texture = EmberGeneratedArt::LoadTexture(EmberGeneratedArt::EnemyTexturePath(MonsterData->Id));
			}
			VisualMID->SetTextureParameterValue(TEXT("Tex"), Texture);
			VisualMID->SetVectorParameterValue(TEXT("Tint"), FLinearColor::White);
			VisualMID->SetScalarParameterValue(TEXT("Flash"), 0.0f);
		}
	}
}

void AEmberCompanionActor::FireProjectile()
{
	if (!MonsterData || !GetWorld())
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
	float BestDistanceSq = FMath::Square(850.0f);
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
		Projectile->Fire(Direction, MonsterData->CompanionDamage, MonsterData->CompanionProjectileSpeed, 0, 0, this, false);
		UE_LOG(LogEmberbound, Log, TEXT("EMBER: CompanionAttack %s damage=%.1f"), *MonsterData->Id.ToString(), MonsterData->CompanionDamage);
	}
}

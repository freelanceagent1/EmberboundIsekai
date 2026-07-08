#include "Combat/EmberProjectile.h"
#include "Combat/HealthComponent.h"
#include "Combat/EmberPlayerCharacter.h"
#include "Core/ObjectPoolSubsystem.h"
#include "Enemies/EmberEnemyBase.h"
#include "Enemies/EnemySpawnerSubsystem.h"
#include "Generated/EmberGeneratedArt.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "EmberboundIsekai.h"

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
	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh)
	{
		Visual->SetStaticMesh(PlaneMesh);
		Visual->SetRelativeScale3D(FVector(0.38f, 1.0f, 0.18f));
	}
}

void AEmberProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	EmberGeneratedArt::FaceComponentToPlayerCamera(Visual, this);
	const FVector Start = GetActorLocation();
	const FVector End = Start + MoveDirection * Speed * DeltaSeconds;
	TraceMovement(Start, End);
	if (!IsActorBeingDestroyed())
	{
		SetActorLocation(End, false);
		PreviousLocation = End;
	}
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
	HitActors.Reset();
	StatusPayload = FEmberProjectileStatusPayload();
}

void AEmberProjectile::Fire(const FVector& Direction, float InDamage, float InSpeed, int32 InPierceCount, int32 InRicochetCount, AActor* InSource, bool bInEnemyProjectile)
{
	MoveDirection = Direction.GetSafeNormal2D();
	if (MoveDirection.IsNearlyZero())
	{
		MoveDirection = GetActorForwardVector();
	}
	Damage = InDamage;
	Speed = InSpeed;
	PierceRemaining = InPierceCount;
	RicochetRemaining = InRicochetCount;
	SourceActor = InSource;
	bEnemyProjectile = bInEnemyProjectile;
	PreviousLocation = GetActorLocation();
	HitActors.Reset();
	SetActorRotation(MoveDirection.Rotation());
	ConfigureVisual(bEnemyProjectile);
}

void AEmberProjectile::ConfigureVisual(bool bForEnemyProjectile)
{
	if (!Visual)
	{
		return;
	}
	const FVector Scale = bForEnemyProjectile ? FVector(0.42f, 1.0f, 0.42f) : FVector(0.5f, 1.0f, 0.2f);
	Visual->SetRelativeScale3D(Scale);
	if (UMaterialInterface* Material = EmberGeneratedArt::LoadBillboardMaterial())
	{
		Visual->SetMaterial(0, Material);
		VisualMID = Visual->CreateAndSetMaterialInstanceDynamic(0);
		if (VisualMID)
		{
			VisualMID->SetTextureParameterValue(TEXT("Tex"), EmberGeneratedArt::LoadTexture(EmberGeneratedArt::ProjectileTexturePath(bForEnemyProjectile)));
			VisualMID->SetVectorParameterValue(TEXT("Tint"), FLinearColor::White);
			VisualMID->SetScalarParameterValue(TEXT("Flash"), bForEnemyProjectile ? 0.2f : 0.0f);
		}
	}
}

void AEmberProjectile::HandleOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	ResolveHit(OtherActor);
}

void AEmberProjectile::TraceMovement(const FVector& From, const FVector& To)
{
	if (!GetWorld())
	{
		return;
	}
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(EmberProjectileTrace), false, this);
	Params.AddIgnoredActor(this);
	if (SourceActor)
	{
		Params.AddIgnoredActor(SourceActor);
	}
	TArray<FHitResult> Hits;
	GetWorld()->SweepMultiByObjectType(Hits, From, To, FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(42.0f), Params);
	for (const FHitResult& Hit : Hits)
	{
		if (AActor* HitActor = Hit.GetActor())
		{
			ResolveHit(HitActor);
			if (!IsValid(this) || IsActorBeingDestroyed())
			{
				return;
			}
		}
	}
}

void AEmberProjectile::ResolveHit(AActor* OtherActor)
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
	if (HitActors.Contains(OtherActor))
	{
		return;
	}
	HitActors.Add(OtherActor);
	if (UHealthComponent* Health = OtherActor->FindComponentByClass<UHealthComponent>())
	{
		Health->ApplyDamage(Damage);
		if (StatusPayload.BurnDps > 0.0f)
		{
			Health->AddDamageStatus(EEmberStatusEffectType::Burn, StatusPayload.BurnDps, StatusPayload.BurnDuration);
		}
		if (StatusPayload.PoisonDps > 0.0f)
		{
			Health->AddDamageStatus(EEmberStatusEffectType::Poison, StatusPayload.PoisonDps, StatusPayload.PoisonDuration);
		}
		if (StatusPayload.FrostSlowFraction > 0.0f)
		{
			Health->AddFrostStatus(StatusPayload.FrostSlowFraction, StatusPayload.FrostDuration);
		}
		UE_LOG(LogEmberbound, Log, TEXT("EMBER: ProjectileHit target=%s damage=%.1f"), *OtherActor->GetName(), Damage);
		if (PierceRemaining > 0)
		{
			--PierceRemaining;
		}
		else if (TryRicochet(OtherActor))
		{
		}
		else
		{
			ReleaseSelf();
		}
	}
}

bool AEmberProjectile::TryRicochet(AActor* HitActor)
{
	if (bEnemyProjectile || RicochetRemaining <= 0 || !GetWorld())
	{
		return false;
	}
	UEnemySpawnerSubsystem* Spawner = GetWorld()->GetSubsystem<UEnemySpawnerSubsystem>();
	if (!Spawner)
	{
		return false;
	}
	AEmberEnemyBase* NewTarget = nullptr;
	float BestDistanceSq = FMath::Square(520.0f);
	for (AEmberEnemyBase* Enemy : Spawner->GetLiveEnemies())
	{
		if (!Enemy || !Enemy->IsAlive() || Enemy == HitActor || HitActors.Contains(Enemy))
		{
			continue;
		}
		const float DistSq = FVector::DistSquared2D(HitActor->GetActorLocation(), Enemy->GetActorLocation());
		if (DistSq < BestDistanceSq)
		{
			BestDistanceSq = DistSq;
			NewTarget = Enemy;
		}
	}
	if (!NewTarget)
	{
		return false;
	}
	--RicochetRemaining;
	MoveDirection = (NewTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	SetActorRotation(MoveDirection.Rotation());
	return true;
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

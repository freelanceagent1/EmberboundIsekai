#include "Enemies/EmberEnemyBase.h"
#include "Combat/EmberPlayerCharacter.h"
#include "Combat/EmberProjectile.h"
#include "Combat/HealthComponent.h"
#include "Combat/LootDropComponent.h"
#include "Core/ObjectPoolSubsystem.h"
#include "Generated/EmberGeneratedArt.h"
#include "Enemies/EnemySpawnerSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "EmberboundIsekai.h"

AEmberEnemyBase::AEmberEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitSphereRadius(44.0f);
	Collision->SetCollisionProfileName(TEXT("Pawn"));
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AEmberEnemyBase::HandleOverlap);

	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(RootComponent);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh)
	{
		Visual->SetStaticMesh(SphereMesh);
		Visual->SetRelativeScale3D(FVector(0.7f));
		Visual->SetHiddenInGame(true);
		Visual->SetVisibility(false);
	}

	BillboardVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BillboardVisual"));
	BillboardVisual->SetupAttachment(RootComponent);
	BillboardVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh)
	{
		BillboardVisual->SetStaticMesh(PlaneMesh);
		BillboardVisual->SetRelativeLocation(FVector(0.0f, 0.0f, 55.0f));
		BillboardVisual->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	}

	BlobShadow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlobShadow"));
	BlobShadow->SetupAttachment(RootComponent);
	BlobShadow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (PlaneMesh)
	{
		BlobShadow->SetStaticMesh(PlaneMesh);
		BlobShadow->SetRelativeLocation(FVector(0.0f, 0.0f, -42.0f));
		BlobShadow->SetRelativeRotation(FRotator::ZeroRotator);
		BlobShadow->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.75f));
	}

	Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	LootDropComponent = CreateDefaultSubobject<ULootDropComponent>(TEXT("LootDropComponent"));
}

void AEmberEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	HealthComponent->OnDied.AddDynamic(this, &AEmberEnemyBase::HandleDeath);
	Player = Cast<AEmberPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RepathTimerHandle, this, &AEmberEnemyBase::UpdateDesiredDirection, 0.25f, true, 0.01f);
	}
	HealthComponent->OnDamaged.AddDynamic(this, &AEmberEnemyBase::HandleDamaged);
}

void AEmberEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RepathTimerHandle);
		World->GetTimerManager().ClearTimer(RangedTimerHandle);
		World->GetTimerManager().ClearTimer(BillboardFlashTimerHandle);
		if (UEnemySpawnerSubsystem* Registry = World->GetSubsystem<UEnemySpawnerSubsystem>())
		{
			Registry->UnregisterEnemy(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}

void AEmberEnemyBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	EmberGeneratedArt::FaceComponentToPlayerCamera(BillboardVisual, this);
	if (IsAlive() && !DesiredDirection.IsNearlyZero())
	{
		const float Speed = RuntimeMoveSpeed * StatusMoveSpeedScale;
		AddActorWorldOffset(DesiredDirection * Speed * DeltaSeconds, true);
	}
}

void AEmberEnemyBase::OnAcquired_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		if (UEnemySpawnerSubsystem* Registry = World->GetSubsystem<UEnemySpawnerSubsystem>())
		{
			Registry->RegisterEnemy(this);
		}
		World->GetTimerManager().SetTimer(RepathTimerHandle, this, &AEmberEnemyBase::UpdateDesiredDirection, 0.25f, true, 0.01f);
	}
	Player = Cast<AEmberPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: EnemySpawned"));
}

void AEmberEnemyBase::OnReleased_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RepathTimerHandle);
		World->GetTimerManager().ClearTimer(RangedTimerHandle);
		if (UEnemySpawnerSubsystem* Registry = World->GetSubsystem<UEnemySpawnerSubsystem>())
		{
			Registry->UnregisterEnemy(this);
		}
	}
	ContactCooldowns.Reset();
	DesiredDirection = FVector::ZeroVector;
	StatusMoveSpeedScale = 1.0f;
	ResetBillboardFlash();
}

void AEmberEnemyBase::InitFromData(UEmberEnemyData* InEnemyData)
{
	EnemyData = InEnemyData;
	if (!EnemyData)
	{
		return;
	}
	RuntimeMoveSpeed = EnemyData->MoveSpeed;
	RuntimeContactDamage = EnemyData->ContactDamage;
	Movement->MaxSpeed = RuntimeMoveSpeed;
	HealthComponent->SetMaxHealth(EnemyData->MaxHP, true);
	LootDropComponent->SetEnemyData(EnemyData);
	Visual->SetRelativeScale3D(EnemyData->VisualScale);
	ConfigureBillboard(EnemyData->GetFName(), EnemyData->VisualScale, EnemyData->ColorTint);
	if (EnemyData->AttackType == EEmberEnemyAttackType::Ranged && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(RangedTimerHandle, this, &AEmberEnemyBase::FireRangedProjectile, EnemyData->ProjectileInterval, true, 1.0f);
	}
}

void AEmberEnemyBase::ApplyRuntimeModifiers(float HealthMultiplier, float DamageMultiplier, float ScaleMultiplier, const FLinearColor& TintOverride)
{
	if (!EnemyData)
	{
		return;
	}
	RuntimeContactDamage = EnemyData->ContactDamage * FMath::Max(0.0f, DamageMultiplier);
	RuntimeMoveSpeed = EnemyData->MoveSpeed;
	Movement->MaxSpeed = RuntimeMoveSpeed;
	HealthComponent->SetMaxHealth(EnemyData->MaxHP * FMath::Max(0.1f, HealthMultiplier), true);
	const FVector ScaledVisual = EnemyData->VisualScale * FMath::Max(0.1f, ScaleMultiplier);
	Visual->SetRelativeScale3D(ScaledVisual);
	ConfigureBillboard(EnemyData->GetFName(), ScaledVisual, TintOverride);
}

void AEmberEnemyBase::ConfigureRuntimeStats(float MaxHealth, float ContactDamage, float MoveSpeed, const FVector& VisualScale, const FLinearColor& Tint)
{
	RuntimeContactDamage = FMath::Max(0.0f, ContactDamage);
	RuntimeMoveSpeed = FMath::Max(0.0f, MoveSpeed);
	Movement->MaxSpeed = RuntimeMoveSpeed;
	HealthComponent->SetMaxHealth(MaxHealth, true);
	Visual->SetRelativeScale3D(VisualScale);
	ConfigureBillboard(TEXT("TutorialDragon"), VisualScale, Tint);
}

bool AEmberEnemyBase::IsAlive() const
{
	return HealthComponent && HealthComponent->IsAlive();
}

void AEmberEnemyBase::SetStatusMoveSpeedScale(float InScale)
{
	StatusMoveSpeedScale = FMath::Clamp(InScale, 0.1f, 1.0f);
}

void AEmberEnemyBase::UpdateDesiredDirection()
{
	if (!Player)
	{
		Player = Cast<AEmberPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	}
	if (!Player)
	{
		DesiredDirection = FVector::ZeroVector;
		return;
	}
	FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.0f;
	const float Distance = ToPlayer.Size();
	DesiredDirection = ToPlayer.GetSafeNormal();
	if (EnemyData && EnemyData->AttackType == EEmberEnemyAttackType::Ranged && Distance < 560.0f)
	{
		DesiredDirection *= -1.0f;
	}
	else if (EnemyData && EnemyData->AttackType == EEmberEnemyAttackType::Ranged && Distance <= 680.0f)
	{
		DesiredDirection = FVector::ZeroVector;
	}
}

void AEmberEnemyBase::ConfigureBillboard(FName TextureId, const FVector& VisualScale, const FLinearColor& Tint)
{
	if (BillboardVisual)
	{
		const float Width = FMath::Clamp(VisualScale.X * 1.35f, 0.6f, 2.6f);
		const float Height = FMath::Clamp(VisualScale.Z * 1.7f, 0.55f, 2.7f);
		BillboardVisual->SetRelativeScale3D(FVector(Width, 1.0f, Height));
		if (UMaterialInterface* Material = EmberGeneratedArt::LoadBillboardMaterial())
		{
			BillboardVisual->SetMaterial(0, Material);
			BillboardMID = BillboardVisual->CreateAndSetMaterialInstanceDynamic(0);
			if (BillboardMID)
			{
				BillboardMID->SetTextureParameterValue(TEXT("Tex"), EmberGeneratedArt::LoadTexture(EmberGeneratedArt::EnemyTexturePath(TextureId)));
				BillboardMID->SetVectorParameterValue(TEXT("Tint"), Tint);
				BillboardMID->SetScalarParameterValue(TEXT("Flash"), 0.0f);
			}
		}
	}
	if (BlobShadow)
	{
		BlobShadow->SetRelativeScale3D(FVector(FMath::Max(0.45f, VisualScale.X), FMath::Max(0.45f, VisualScale.Y), 1.0f));
		if (UMaterialInterface* ShadowMaterial = EmberGeneratedArt::LoadBlobShadowMaterial())
		{
			BlobShadow->SetMaterial(0, ShadowMaterial);
		}
	}
}

void AEmberEnemyBase::ResetBillboardFlash()
{
	if (BillboardMID)
	{
		BillboardMID->SetScalarParameterValue(TEXT("Flash"), 0.0f);
	}
}

void AEmberEnemyBase::FireRangedProjectile()
{
	if (!Player || !EnemyData || !GetWorld() || !IsAlive())
	{
		return;
	}
	const FVector Direction = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	if (UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>())
	{
		const FVector SpawnLocation = GetActorLocation() + Direction * 60.0f + FVector(0.0f, 0.0f, 45.0f);
		if (AEmberProjectile* Projectile = Pool->Acquire<AEmberProjectile>(AEmberProjectile::StaticClass(), FTransform(Direction.Rotation(), SpawnLocation)))
		{
			Projectile->Fire(Direction, EnemyData->ProjectileDamage, EnemyData->ProjectileSpeed, 0, 0, this, true);
		}
	}
}

void AEmberEnemyBase::HandleOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (RuntimeContactDamage <= 0.0f || !OtherActor || !OtherActor->IsA<AEmberPlayerCharacter>())
	{
		return;
	}
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const float* NextAllowed = ContactCooldowns.Find(OtherActor);
	if (NextAllowed && *NextAllowed > Now)
	{
		return;
	}
	ContactCooldowns.Add(OtherActor, Now + 0.8f);
	if (UHealthComponent* Health = OtherActor->FindComponentByClass<UHealthComponent>())
	{
		Health->ApplyDamage(RuntimeContactDamage);
	}
}

void AEmberEnemyBase::HandleDamaged(float, float)
{
	if (BillboardMID)
	{
		BillboardMID->SetScalarParameterValue(TEXT("Flash"), 1.0f);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(BillboardFlashTimerHandle, this, &AEmberEnemyBase::ResetBillboardFlash, 0.08f, false);
	}
}

void AEmberEnemyBase::HandleDeath(AActor* DeadActor)
{
	if (LootDropComponent)
	{
		LootDropComponent->HandleOwnerDied(DeadActor);
	}
	if (UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>())
	{
		Pool->ReleaseActor(this);
	}
	else
	{
		Destroy();
	}
}

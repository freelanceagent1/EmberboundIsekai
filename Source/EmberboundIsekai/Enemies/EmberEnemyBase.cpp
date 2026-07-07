#include "Enemies/EmberEnemyBase.h"
#include "Combat/EmberPlayerCharacter.h"
#include "Combat/EmberProjectile.h"
#include "Combat/HealthComponent.h"
#include "Combat/LootDropComponent.h"
#include "Core/ObjectPoolSubsystem.h"
#include "Enemies/EnemySpawnerSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"
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
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Visual->SetStaticMesh(SphereMesh.Object);
		Visual->SetRelativeScale3D(FVector(0.7f));
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
}

void AEmberEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
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
	Super::EndPlay(EndPlayReason);
}

void AEmberEnemyBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (IsAlive() && !DesiredDirection.IsNearlyZero())
	{
		const float Speed = (EnemyData ? EnemyData->MoveSpeed : 250.0f) * StatusMoveSpeedScale;
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
}

void AEmberEnemyBase::InitFromData(UEmberEnemyData* InEnemyData)
{
	EnemyData = InEnemyData;
	if (!EnemyData)
	{
		return;
	}
	Movement->MaxSpeed = EnemyData->MoveSpeed;
	HealthComponent->SetMaxHealth(EnemyData->MaxHP, true);
	LootDropComponent->SetEnemyData(EnemyData);
	Visual->SetRelativeScale3D(EnemyData->VisualScale);
	if (UMaterialInterface* EnemyMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Emberbound/Materials/Stylized/M_Enemy.M_Enemy")))
	{
		Visual->SetMaterial(0, EnemyMaterial);
	}
	if (UMaterialInstanceDynamic* MID = Visual->CreateAndSetMaterialInstanceDynamic(0))
	{
		MID->SetVectorParameterValue(TEXT("Tint"), EnemyData->ColorTint);
		MID->SetScalarParameterValue(TEXT("Flash"), 0.0f);
	}
	if (EnemyData->AttackType == EEmberEnemyAttackType::Ranged && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(RangedTimerHandle, this, &AEmberEnemyBase::FireRangedProjectile, EnemyData->ProjectileInterval, true, 1.0f);
	}
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
	if (!EnemyData || !OtherActor || !OtherActor->IsA<AEmberPlayerCharacter>())
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
		Health->ApplyDamage(EnemyData->ContactDamage);
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

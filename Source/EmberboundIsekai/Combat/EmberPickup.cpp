#include "Combat/EmberPickup.h"
#include "Combat/EmberPlayerCharacter.h"
#include "Combat/XPComponent.h"
#include "Core/EconomySubsystem.h"
#include "Core/ObjectPoolSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"

AEmberPickup::AEmberPickup()
{
	PrimaryActorTick.bCanEverTick = true;
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitSphereRadius(28.0f);
	Collision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AEmberPickup::HandleOverlap);

	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(RootComponent);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Visual->SetStaticMesh(SphereMesh.Object);
		Visual->SetRelativeScale3D(FVector(0.12f));
	}
}

void AEmberPickup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	AEmberPlayerCharacter* Player = Cast<AEmberPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Player)
	{
		return;
	}
	const float Distance = FVector::Dist2D(GetActorLocation(), Player->GetActorLocation());
	if (Distance <= Player->GetPickupRadius())
	{
		const FVector NewLocation = FMath::VInterpTo(GetActorLocation(), Player->GetActorLocation(), DeltaSeconds, 8.0f);
		SetActorLocation(NewLocation);
	}
}

void AEmberPickup::OnReleased_Implementation()
{
	Amount = 1;
}

void AEmberPickup::Configure(EEmberPickupType InType, int32 InAmount)
{
	PickupType = InType;
	Amount = FMath::Max(1, InAmount);
}

void AEmberPickup::HandleOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (AEmberPlayerCharacter* Player = Cast<AEmberPlayerCharacter>(OtherActor))
	{
		Collect(Player);
	}
}

void AEmberPickup::Collect(AEmberPlayerCharacter* Player)
{
	if (!Player)
	{
		return;
	}
	if (PickupType == EEmberPickupType::XP)
	{
		if (UXPComponent* XPComponent = Player->FindComponentByClass<UXPComponent>())
		{
			XPComponent->AddXP(Amount);
		}
	}
	else if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UEconomySubsystem* Economy = GameInstance->GetSubsystem<UEconomySubsystem>())
		{
			const EEmberResource Resource = PickupType == EEmberPickupType::Wood ? EEmberResource::Wood : PickupType == EEmberPickupType::SoulAsh ? EEmberResource::SoulAsh : EEmberResource::Gold;
			Economy->Add(Resource, Amount);
		}
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

#include "Combat/EmberPickup.h"
#include "Combat/EmberPlayerCharacter.h"
#include "Combat/XPComponent.h"
#include "Core/EmberAudioSubsystem.h"
#include "Core/EconomySubsystem.h"
#include "Core/ObjectPoolSubsystem.h"
#include "Expedition/ExpeditionGameMode.h"
#include "Generated/EmberGeneratedArt.h"
#include "Run/RunUpgradeManager.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

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
	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh)
	{
		Visual->SetStaticMesh(PlaneMesh);
		Visual->SetRelativeLocation(FVector(0.0f, 0.0f, 26.0f));
		Visual->SetRelativeScale3D(FVector(0.36f, 0.36f, 1.0f));
	}
}

void AEmberPickup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	EmberGeneratedArt::FaceComponentToPlayerCamera(Visual, this);
	AEmberPlayerCharacter* Player = Cast<AEmberPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Player)
	{
		return;
	}
	if (UEmberAudioSubsystem* Audio = GetGameInstance() ? GetGameInstance()->GetSubsystem<UEmberAudioSubsystem>() : nullptr)
	{
		Audio->PlaySfx(TEXT("pickup"), this);
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
	ConfigureVisual();
}

void AEmberPickup::ConfigureVisual()
{
	if (!Visual)
	{
		return;
	}
	FName TextureId = TEXT("xp");
	if (PickupType == EEmberPickupType::Gold)
	{
		TextureId = TEXT("gold");
	}
	else if (PickupType == EEmberPickupType::SoulAsh)
	{
		TextureId = TEXT("soul-ash");
	}
	else if (PickupType == EEmberPickupType::Wood)
	{
		TextureId = TEXT("wood");
	}
	if (UMaterialInterface* Material = EmberGeneratedArt::LoadBillboardMaterial())
	{
		Visual->SetMaterial(0, Material);
		VisualMID = Visual->CreateAndSetMaterialInstanceDynamic(0);
		if (VisualMID)
		{
			const FString TexturePath = (PickupType == EEmberPickupType::Gold || PickupType == EEmberPickupType::SoulAsh)
				? EmberGeneratedArt::PickupTexturePath(TextureId)
				: EmberGeneratedArt::HudIconTexturePath(TextureId);
			VisualMID->SetTextureParameterValue(TEXT("Tex"), EmberGeneratedArt::LoadTexture(TexturePath));
			VisualMID->SetVectorParameterValue(TEXT("Tint"), FLinearColor::White);
			VisualMID->SetScalarParameterValue(TEXT("Flash"), 0.0f);
		}
	}
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
	else
	{
		const EEmberResource Resource = PickupType == EEmberPickupType::Wood ? EEmberResource::Wood : PickupType == EEmberPickupType::SoulAsh ? EEmberResource::SoulAsh : EEmberResource::Gold;
		int32 FinalAmount = Amount;
		if (Resource == EEmberResource::Gold)
		{
			if (URunUpgradeManager* UpgradeManager = GetWorld()->GetSubsystem<URunUpgradeManager>())
			{
				FinalAmount = FMath::Max(1, FMath::RoundToInt(static_cast<float>(Amount) * UpgradeManager->GetGoldMultiplier()));
			}
		}
		if (AExpeditionGameMode* ExpeditionMode = GetWorld() ? GetWorld()->GetAuthGameMode<AExpeditionGameMode>() : nullptr)
		{
			ExpeditionMode->AddRunResource(Resource, FinalAmount);
		}
		else if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (UEconomySubsystem* Economy = GameInstance->GetSubsystem<UEconomySubsystem>())
			{
				Economy->Add(Resource, FinalAmount);
			}
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

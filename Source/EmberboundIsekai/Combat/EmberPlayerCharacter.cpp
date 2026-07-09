#include "Combat/EmberPlayerCharacter.h"
#include "Combat/AutoAttackComponent.h"
#include "Combat/HealthComponent.h"
#include "Combat/XPComponent.h"
#include "Generated/EmberGeneratedArt.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "EmberboundIsekai.h"

AEmberPlayerCharacter::AEmberPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
	GetCharacterMovement()->MaxWalkSpeed = 450.0f;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);
	Camera->bUsePawnControlRotation = false;
	Camera->SetProjectionMode(ECameraProjectionMode::Orthographic);
	Camera->SetOrthoWidth(1900.0f);
	Camera->SetRelativeLocation(FVector(0.0f, 0.0f, 2200.0f));
	Camera->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->MaxHP = 100.0f;
	XPComponent = CreateDefaultSubobject<UXPComponent>(TEXT("XPComponent"));
	AutoAttackComponent = CreateDefaultSubobject<UAutoAttackComponent>(TEXT("AutoAttackComponent"));

	BillboardVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BillboardVisual"));
	BillboardVisual->SetupAttachment(RootComponent);
	BillboardVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh)
	{
		BillboardVisual->SetStaticMesh(PlaneMesh);
		BillboardVisual->SetRelativeLocation(FVector(0.0f, 0.0f, 88.0f));
		BillboardVisual->SetRelativeScale3D(FVector(2.05f, 2.75f, 1.0f));
	}

	BlobShadow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlobShadow"));
	BlobShadow->SetupAttachment(RootComponent);
	BlobShadow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (PlaneMesh)
	{
		BlobShadow->SetStaticMesh(PlaneMesh);
		BlobShadow->SetRelativeLocation(FVector(0.0f, 0.0f, -92.0f));
		BlobShadow->SetRelativeScale3D(FVector(0.8f, 0.8f, 1.0f));
	}
}

void AEmberPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	BuildRuntimeInput();
	if (UMaterialInterface* Material = EmberGeneratedArt::LoadBillboardMaterial())
	{
		BillboardVisual->SetMaterial(0, Material);
		if (UMaterialInstanceDynamic* MID = BillboardVisual->CreateAndSetMaterialInstanceDynamic(0))
		{
			MID->SetTextureParameterValue(TEXT("Tex"), EmberGeneratedArt::LoadTexture(TEXT("/Game/Emberbound/Generated/Characters/T_RiftArcherFull.T_RiftArcherFull")));
			MID->SetVectorParameterValue(TEXT("Tint"), FLinearColor::White);
			MID->SetScalarParameterValue(TEXT("Flash"), 0.0f);
		}
	}
	if (UMaterialInterface* ShadowMaterial = EmberGeneratedArt::LoadBlobShadowMaterial())
	{
		BlobShadow->SetMaterial(0, ShadowMaterial);
	}
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: PlayerSpawned"));
}

void AEmberPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	EmberGeneratedArt::FaceComponentToPlayerCamera(BillboardVisual, this);
}

void AEmberPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (!MoveAction)
		{
			BuildRuntimeInput();
		}
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AEmberPlayerCharacter::Move);
	}
}

void AEmberPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Movement = Value.Get<FVector2D>();
	ApplyVirtualJoystickVector(Movement);
}

void AEmberPlayerCharacter::ApplyPickupRadiusMultiplier(float Multiplier)
{
	PickupRadius *= FMath::Max(1.0f, Multiplier);
}

void AEmberPlayerCharacter::SetAutoAttackEnabled(bool bEnabled)
{
	if (AutoAttackComponent)
	{
		AutoAttackComponent->SetAttackingEnabled(bEnabled);
	}
}

void AEmberPlayerCharacter::ApplyVirtualJoystickVector(const FVector2D& Movement)
{
	if (!Movement.IsNearlyZero())
	{
		const FVector2D Clamped = Movement.GetClampedToMaxSize(1.0f);
		AddMovementInput(FVector::ForwardVector, Clamped.Y);
		AddMovementInput(FVector::RightVector, Clamped.X);
	}
}

void AEmberPlayerCharacter::BuildRuntimeInput()
{
	if (!MoveAction)
	{
		MoveAction = NewObject<UInputAction>(this, TEXT("IA_RuntimeMove"));
		MoveAction->ValueType = EInputActionValueType::Axis2D;
	}
	if (!MappingContext)
	{
		MappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_RuntimeMove"));
		UInputModifierSwizzleAxis* SwizzleYXZ = NewObject<UInputModifierSwizzleAxis>(MappingContext);
		SwizzleYXZ->Order = EInputAxisSwizzle::YXZ;
		UInputModifierNegate* Negate = NewObject<UInputModifierNegate>(MappingContext);
		Negate->bX = true;
		Negate->bY = true;
		Negate->bZ = false;
		UInputModifierNegate* NegateX = NewObject<UInputModifierNegate>(MappingContext);
		NegateX->bX = true;
		NegateX->bY = false;
		NegateX->bZ = false;

		MappingContext->MapKey(MoveAction, EKeys::W).Modifiers.Add(SwizzleYXZ);
		MappingContext->MapKey(MoveAction, EKeys::S).Modifiers.Append({ SwizzleYXZ, Negate });
		MappingContext->MapKey(MoveAction, EKeys::D);
		MappingContext->MapKey(MoveAction, EKeys::A).Modifiers.Add(NegateX);
		MappingContext->MapKey(MoveAction, EKeys::Gamepad_Left2D);
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				Subsystem->AddMappingContext(MappingContext, 0);
			}
		}
	}
}

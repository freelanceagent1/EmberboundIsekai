#include "Combat/EmberPlayerCharacter.h"
#include "Combat/AutoAttackComponent.h"
#include "Combat/HealthComponent.h"
#include "Combat/XPComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "EmberboundIsekai.h"

AEmberPlayerCharacter::AEmberPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
	GetCharacterMovement()->MaxWalkSpeed = 450.0f;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 1400.0f;
	SpringArm->SetRelativeRotation(FRotator(-55.0f, 0.0f, 0.0f));
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bEnableCameraRotationLag = false;
	SpringArm->bEnableCameraLag = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->MaxHP = 100.0f;
	XPComponent = CreateDefaultSubobject<UXPComponent>(TEXT("XPComponent"));
	AutoAttackComponent = CreateDefaultSubobject<UAutoAttackComponent>(TEXT("AutoAttackComponent"));
}

void AEmberPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	BuildRuntimeInput();
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: PlayerSpawned"));
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
	if (!Movement.IsNearlyZero())
	{
		AddMovementInput(FVector::ForwardVector, Movement.Y);
		AddMovementInput(FVector::RightVector, Movement.X);
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

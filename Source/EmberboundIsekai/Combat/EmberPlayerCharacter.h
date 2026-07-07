#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "EmberPlayerCharacter.generated.h"

class UAutoAttackComponent;
class UCameraComponent;
class UHealthComponent;
class USpringArmComponent;
class UXPComponent;

UCLASS()
class EMBERBOUNDISEKAI_API AEmberPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEmberPlayerCharacter();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	float GetPickupRadius() const { return PickupRadius; }

	UHealthComponent* GetHealthComponent() const { return HealthComponent; }
	UXPComponent* GetXPComponent() const { return XPComponent; }

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UXPComponent> XPComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAutoAttackComponent> AutoAttackComponent;

	UPROPERTY()
	TObjectPtr<class UInputAction> MoveAction;

	UPROPERTY()
	TObjectPtr<class UInputMappingContext> MappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float PickupRadius = 180.0f;

	void Move(const FInputActionValue& Value);
	void BuildRuntimeInput();
};

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "XPComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEmberLevelUp, int32, NewLevel);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class EMBERBOUNDISEKAI_API UXPComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UXPComponent();

	UFUNCTION(BlueprintCallable, Category = "XP")
	void AddXP(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "XP")
	int32 GetLevel() const { return Level; }

	UFUNCTION(BlueprintCallable, Category = "XP")
	int32 GetXP() const { return XP; }

	UFUNCTION(BlueprintCallable, Category = "XP")
	int32 GetXPToNext() const;

	UPROPERTY(BlueprintAssignable, Category = "XP")
	FOnEmberLevelUp OnLevelUp;

private:
	UPROPERTY()
	int32 XP = 0;

	UPROPERTY()
	int32 Level = 1;
};

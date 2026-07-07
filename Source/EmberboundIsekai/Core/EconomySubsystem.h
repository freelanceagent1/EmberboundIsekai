#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/ResourceTypes.h"
#include "EconomySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmberResourceChanged, EEmberResource, Resource, int32, NewAmount);

UCLASS()
class EMBERBOUNDISEKAI_API UEconomySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Economy")
	void Add(EEmberResource Resource, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Economy")
	bool Spend(EEmberResource Resource, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Economy")
	int32 Get(EEmberResource Resource) const;

	UPROPERTY(BlueprintAssignable, Category = "Economy")
	FOnEmberResourceChanged OnResourceChanged;

private:
	UPROPERTY()
	TMap<EEmberResource, int32> Wallet;
};

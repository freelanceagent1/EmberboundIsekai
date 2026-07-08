#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EmberDataRegistry.generated.h"

class UPrimaryDataAsset;

UCLASS()
class EMBERBOUNDISEKAI_API UEmberDataRegistry : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Data")
	UPrimaryDataAsset* FindDataAsset(FName Type, FName Name) const;

	template <typename T>
	T* FindDataAsset(FName Name) const
	{
		return Cast<T>(FindDataAsset(T::StaticClass()->GetFName(), Name));
	}

private:
	UPROPERTY()
	TMap<FName, UPrimaryDataAsset*> AssetsByTypeAndName;

	void RegisterAsset(UPrimaryDataAsset* Asset);
	static FName MakeKey(FName Type, FName Name);
};

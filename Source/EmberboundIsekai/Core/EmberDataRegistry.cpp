#include "Core/EmberDataRegistry.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/DataAsset.h"
#include "EmberboundIsekai.h"

void UEmberDataRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	AssetRegistry.ScanPathsSynchronous({ TEXT("/Game/Emberbound/Data") }, true);

	FARFilter Filter;
	Filter.PackagePaths.Add(TEXT("/Game/Emberbound/Data"));
	Filter.bRecursivePaths = true;
	Filter.ClassPaths.Add(UPrimaryDataAsset::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;

	TArray<FAssetData> Assets;
	AssetRegistry.GetAssets(Filter, Assets);
	for (const FAssetData& AssetData : Assets)
	{
		if (UPrimaryDataAsset* Asset = Cast<UPrimaryDataAsset>(AssetData.GetAsset()))
		{
			const FPrimaryAssetId Id = Asset->GetPrimaryAssetId();
			const FName Type = Id.PrimaryAssetType.IsValid() ? Id.PrimaryAssetType.GetName() : Asset->GetClass()->GetFName();
			AssetsByTypeAndName.Add(MakeKey(Type, Asset->GetFName()), Asset);
			AssetsByTypeAndName.Add(MakeKey(Asset->GetClass()->GetFName(), Asset->GetFName()), Asset);
		}
	}
	UE_LOG(LogEmberbound, Log, TEXT("EMBER: DataRegistryLoaded Count=%d"), AssetsByTypeAndName.Num());
}

UPrimaryDataAsset* UEmberDataRegistry::FindDataAsset(FName Type, FName Name) const
{
	if (UPrimaryDataAsset* const* Asset = AssetsByTypeAndName.Find(MakeKey(Type, Name)))
	{
		return *Asset;
	}
	return nullptr;
}

FName UEmberDataRegistry::MakeKey(FName Type, FName Name)
{
	return FName(FString::Printf(TEXT("%s:%s"), *Type.ToString(), *Name.ToString()));
}

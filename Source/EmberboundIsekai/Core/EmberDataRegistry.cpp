#include "Core/EmberDataRegistry.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
#include "Data/BuildingData.h"
#include "Data/EmberEnemyData.h"
#include "Data/MonsterData.h"
#include "Data/RunUpgradeData.h"
#include "Data/TitleData.h"
#include "Engine/DataAsset.h"
#include "EmberboundIsekai.h"

namespace
{
	template <typename T>
	void LoadKnownAssets(const TArray<const TCHAR*>& ObjectPaths, TArray<UPrimaryDataAsset*>& OutAssets)
	{
		for (const TCHAR* ObjectPath : ObjectPaths)
		{
			if (T* Asset = LoadObject<T>(nullptr, ObjectPath))
			{
				OutAssets.Add(Asset);
			}
		}
	}

	TArray<UPrimaryDataAsset*> LoadKnownEmberDataAssets()
	{
		TArray<UPrimaryDataAsset*> Assets;
		LoadKnownAssets<UBuildingData>({
			TEXT("/Game/Emberbound/Data/Buildings/DA_CampfireCore.DA_CampfireCore"),
			TEXT("/Game/Emberbound/Data/Buildings/DA_GrimForge.DA_GrimForge"),
			TEXT("/Game/Emberbound/Data/Buildings/DA_MonsterPen.DA_MonsterPen"),
			TEXT("/Game/Emberbound/Data/Buildings/DA_Watchtower.DA_Watchtower")
		}, Assets);
		LoadKnownAssets<UEmberEnemyData>({
			TEXT("/Game/Emberbound/Data/Enemies/BoneRat.BoneRat"),
			TEXT("/Game/Emberbound/Data/Enemies/GoblinArcher.GoblinArcher"),
			TEXT("/Game/Emberbound/Data/Enemies/GoblinStabber.GoblinStabber"),
			TEXT("/Game/Emberbound/Data/Enemies/GreenSlime.GreenSlime"),
			TEXT("/Game/Emberbound/Data/Enemies/RedSlime.RedSlime")
		}, Assets);
		LoadKnownAssets<UMonsterData>({
			TEXT("/Game/Emberbound/Data/Monsters/DA_BoneRat.DA_BoneRat"),
			TEXT("/Game/Emberbound/Data/Monsters/DA_SkeletonArcher.DA_SkeletonArcher"),
			TEXT("/Game/Emberbound/Data/Monsters/DA_Slime.DA_Slime")
		}, Assets);
		LoadKnownAssets<UTitleData>({
			TEXT("/Game/Emberbound/Data/Titles/DA_Title_FailedChosenOne.DA_Title_FailedChosenOne"),
			TEXT("/Game/Emberbound/Data/Titles/DA_Title_FormerOfficeDrone.DA_Title_FormerOfficeDrone"),
			TEXT("/Game/Emberbound/Data/Titles/DA_Title_GenreSavvyIdiot.DA_Title_GenreSavvyIdiot"),
			TEXT("/Game/Emberbound/Data/Titles/DA_Title_SpreadsheetHero.DA_Title_SpreadsheetHero"),
			TEXT("/Game/Emberbound/Data/Titles/DA_Title_TruckKunSurvivor.DA_Title_TruckKunSurvivor")
		}, Assets);
		LoadKnownAssets<URunUpgradeData>({
			TEXT("/Game/Emberbound/Data/Upgrades/DA_BoneShield.DA_BoneShield"),
			TEXT("/Game/Emberbound/Data/Upgrades/DA_CompanionSlime.DA_CompanionSlime"),
			TEXT("/Game/Emberbound/Data/Upgrades/DA_CritFocus.DA_CritFocus"),
			TEXT("/Game/Emberbound/Data/Upgrades/DA_FireArrow.DA_FireArrow"),
			TEXT("/Game/Emberbound/Data/Upgrades/DA_FrostArrow.DA_FrostArrow"),
			TEXT("/Game/Emberbound/Data/Upgrades/DA_GoldRush.DA_GoldRush"),
			TEXT("/Game/Emberbound/Data/Upgrades/DA_HealingWisp.DA_HealingWisp"),
			TEXT("/Game/Emberbound/Data/Upgrades/DA_HeavyArrow.DA_HeavyArrow"),
			TEXT("/Game/Emberbound/Data/Upgrades/DA_PickupMagnet.DA_PickupMagnet"),
			TEXT("/Game/Emberbound/Data/Upgrades/DA_PiercingShot.DA_PiercingShot"),
			TEXT("/Game/Emberbound/Data/Upgrades/DA_PoisonArrow.DA_PoisonArrow"),
			TEXT("/Game/Emberbound/Data/Upgrades/DA_RapidFire.DA_RapidFire"),
			TEXT("/Game/Emberbound/Data/Upgrades/DA_RicochetShot.DA_RicochetShot"),
			TEXT("/Game/Emberbound/Data/Upgrades/DA_SoulFlameTrail.DA_SoulFlameTrail"),
			TEXT("/Game/Emberbound/Data/Upgrades/DA_SplitShot.DA_SplitShot")
		}, Assets);
		return Assets;
	}
}

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
			RegisterAsset(Asset);
		}
	}
	if (AssetsByTypeAndName.Num() == 0)
	{
		for (UPrimaryDataAsset* Asset : LoadKnownEmberDataAssets())
		{
			RegisterAsset(Asset);
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

void UEmberDataRegistry::RegisterAsset(UPrimaryDataAsset* Asset)
{
	if (!Asset)
	{
		return;
	}
	const FPrimaryAssetId Id = Asset->GetPrimaryAssetId();
	const FName Type = Id.PrimaryAssetType.IsValid() ? Id.PrimaryAssetType.GetName() : Asset->GetClass()->GetFName();
	AssetsByTypeAndName.Add(MakeKey(Type, Asset->GetFName()), Asset);
	AssetsByTypeAndName.Add(MakeKey(Asset->GetClass()->GetFName(), Asset->GetFName()), Asset);
}

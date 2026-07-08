#include "Generated/EmberGeneratedArt.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/SceneComponent.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Math/RotationMatrix.h"

namespace
{
	const TCHAR* BillboardMaterialPath = TEXT("/Game/Emberbound/Materials/Generated/M_Generated_Billboard.M_Generated_Billboard");
	const TCHAR* CampfireFlameMaterialPath = TEXT("/Game/Emberbound/Materials/Generated/M_Campfire_Flame.M_Campfire_Flame");
	const TCHAR* BlobShadowMaterialPath = TEXT("/Game/Emberbound/Materials/Generated/M_BlobShadow.M_BlobShadow");

	FString TexturePath(const TCHAR* Category, const FString& TextureName)
	{
		return EmberGeneratedArt::TextureObjectPath(FString::Printf(TEXT("/Game/Emberbound/Generated/%s/%s"), Category, *TextureName));
	}
}

namespace EmberGeneratedArt
{
	FString TextureObjectPath(const FString& PackagePath)
	{
		FString AssetName;
		PackagePath.Split(TEXT("/"), nullptr, &AssetName, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
		return FString::Printf(TEXT("%s.%s"), *PackagePath, *AssetName);
	}

	FString PascalFromKebab(const FString& Id)
	{
		TArray<FString> Parts;
		Id.ParseIntoArray(Parts, TEXT("-"), true);
		FString Result;
		for (FString Part : Parts)
		{
			if (Part.IsEmpty())
			{
				continue;
			}
			Part = Part.ToLower();
			Part[0] = FChar::ToUpper(Part[0]);
			Result += Part;
		}
		return Result;
	}

	FString EnemyTexturePath(FName EnemyId)
	{
		const TMap<FName, FString> Paths = {
			{ TEXT("GreenSlime"), TEXT("T_GreenSlime") },
			{ TEXT("RedSlime"), TEXT("T_RedSlime") },
			{ TEXT("GoblinStabber"), TEXT("T_GoblinStabber") },
			{ TEXT("GoblinArcher"), TEXT("T_GoblinArcher") },
			{ TEXT("BoneRat"), TEXT("T_BoneRat") },
			{ TEXT("TutorialDragon"), TEXT("T_TutorialDragon") }
		};
		const FString* TextureName = Paths.Find(EnemyId);
		return TexturePath(TEXT("Characters"), TextureName ? *TextureName : TEXT("T_GreenSlime"));
	}

	FString BuildingTexturePath(FName BuildingId)
	{
		const TMap<FName, FString> Paths = {
			{ TEXT("campfire-core"), TEXT("T_CampfireCore") },
			{ TEXT("grim-forge"), TEXT("T_GrimForge") },
			{ TEXT("monster-pen"), TEXT("T_MonsterPen") },
			{ TEXT("watchtower"), TEXT("T_Watchtower") },
			{ TEXT("expedition-portal"), TEXT("T_ExpeditionPortal") }
		};
		const FString* TextureName = Paths.Find(BuildingId);
		if (TextureName)
		{
			return TexturePath(TEXT("Buildings"), *TextureName);
		}
		const FString FallbackTextureName = FString::Printf(TEXT("T_%s"), *PascalFromKebab(BuildingId.ToString()));
		return TexturePath(TEXT("Buildings"), FallbackTextureName);
	}

	FString PickupTexturePath(FName PickupId)
	{
		const FString TextureName = FString::Printf(TEXT("T_%sPickup"), *PascalFromKebab(PickupId.ToString()));
		return TexturePath(TEXT("VFX"), TextureName);
	}

	FString ProjectileTexturePath(bool bEnemyProjectile)
	{
		return TexturePath(TEXT("VFX"), bEnemyProjectile ? TEXT("T_FireProjectile") : TEXT("T_ArrowProjectile"));
	}

	FString UpgradeIconTexturePath(FName UpgradeId)
	{
		return TexturePath(TEXT("UpgradeIcons"), FString::Printf(TEXT("T_%s"), *PascalFromKebab(UpgradeId.ToString())));
	}

	FString HudIconTexturePath(FName IconId)
	{
		return TexturePath(TEXT("UIIcons"), FString::Printf(TEXT("T_%s"), *PascalFromKebab(IconId.ToString())));
	}

	TArray<FName> HudIconIds()
	{
		return {
			TEXT("health"),
			TEXT("xp"),
			TEXT("gold"),
			TEXT("wood"),
			TEXT("soul-ash"),
			TEXT("embers")
		};
	}

	TArray<FName> UpgradeIconIds()
	{
		return {
			TEXT("split-shot"),
			TEXT("piercing-shot"),
			TEXT("ricochet-shot"),
			TEXT("rapid-fire"),
			TEXT("heavy-arrow"),
			TEXT("fire-arrow"),
			TEXT("poison-arrow"),
			TEXT("frost-arrow"),
			TEXT("bone-shield"),
			TEXT("healing-wisp"),
			TEXT("pickup-magnet"),
			TEXT("crit-focus"),
			TEXT("companion-slime"),
			TEXT("soul-flame-trail"),
			TEXT("gold-rush")
		};
	}

	UTexture2D* LoadTexture(const FString& ObjectPath)
	{
		return LoadObject<UTexture2D>(nullptr, *ObjectPath);
	}

	UMaterialInterface* LoadBillboardMaterial()
	{
		return LoadObject<UMaterialInterface>(nullptr, BillboardMaterialPath);
	}

	UMaterialInterface* LoadCampfireFlameMaterial()
	{
		return LoadObject<UMaterialInterface>(nullptr, CampfireFlameMaterialPath);
	}

	UMaterialInterface* LoadBlobShadowMaterial()
	{
		return LoadObject<UMaterialInterface>(nullptr, BlobShadowMaterialPath);
	}

	void FaceComponentToPlayerCamera(USceneComponent* Component, const UObject* WorldContextObject)
	{
		if (!Component || !WorldContextObject)
		{
			return;
		}
		const APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
		if (PC && PC->PlayerCameraManager)
		{
			const FVector ToCamera = (PC->PlayerCameraManager->GetCameraLocation() - Component->GetComponentLocation()).GetSafeNormal();
			if (!ToCamera.IsNearlyZero())
			{
				const FVector CameraRight = FRotationMatrix(PC->PlayerCameraManager->GetCameraRotation()).GetUnitAxis(EAxis::Y);
				Component->SetWorldRotation(FRotationMatrix::MakeFromXZ(CameraRight, ToCamera).Rotator());
			}
		}
	}
}

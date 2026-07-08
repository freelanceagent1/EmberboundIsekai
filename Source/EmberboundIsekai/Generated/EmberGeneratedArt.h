#pragma once

#include "CoreMinimal.h"

class UMaterialInterface;
class USceneComponent;
class UTexture2D;

namespace EmberGeneratedArt
{
	FString TextureObjectPath(const FString& PackagePath);
	FString PascalFromKebab(const FString& Id);
	FString EnemyTexturePath(FName EnemyId);
	FString BuildingTexturePath(FName BuildingId);
	FString PickupTexturePath(FName PickupId);
	FString ProjectileTexturePath(bool bEnemyProjectile);
	FString UpgradeIconTexturePath(FName UpgradeId);
	FString HudIconTexturePath(FName IconId);
	TArray<FName> HudIconIds();
	TArray<FName> UpgradeIconIds();

	UTexture2D* LoadTexture(const FString& ObjectPath);
	UMaterialInterface* LoadBillboardMaterial();
	UMaterialInterface* LoadCampfireFlameMaterial();
	UMaterialInterface* LoadBlobShadowMaterial();
	void FaceComponentToPlayerCamera(USceneComponent* Component, const UObject* WorldContextObject);
}

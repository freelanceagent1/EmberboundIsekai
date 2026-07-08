#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/Texture2D.h"
#include "ResourceTypes.h"
#include "MonsterData.generated.h"

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EMonsterRoleFlags : uint8
{
	None = 0 UMETA(Hidden),
	Companion = 1 << 0,
	Worker = 1 << 1,
	Defender = 1 << 2
};
ENUM_CLASS_FLAGS(EMonsterRoleFlags);

UENUM(BlueprintType)
enum class EMonsterAssignmentRole : uint8
{
	None,
	Companion,
	Worker,
	Defender
};

USTRUCT(BlueprintType)
struct EMBERBOUNDISEKAI_API FMonsterWorkerOutput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Worker")
	EEmberResource Resource = EEmberResource::Gold;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Worker", meta = (ClampMin = "0.0"))
	float PerMinute = 0.0f;
};

UCLASS(BlueprintType)
class EMBERBOUNDISEKAI_API UMonsterData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster", meta = (ClampMin = "1"))
	int32 ShardsToUnlock = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster", meta = (Bitmask, BitmaskEnum = "/Script/EmberboundIsekai.EMonsterRoleFlags"))
	int32 AllowedRoles = static_cast<int32>(EMonsterRoleFlags::Companion);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Worker")
	TArray<FMonsterWorkerOutput> WorkerOutputs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Companion", meta = (ClampMin = "0.0"))
	float CompanionDamage = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Companion", meta = (ClampMin = "0.1"))
	float CompanionAttackInterval = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Companion", meta = (ClampMin = "0.0"))
	float CompanionProjectileSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defender", meta = (ClampMin = "0.0"))
	float DefenderDps = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TSoftObjectPtr<UTexture2D> IconTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TSoftObjectPtr<UTexture2D> BillboardTexture;

	bool AllowsRole(EMonsterAssignmentRole Role) const;
};

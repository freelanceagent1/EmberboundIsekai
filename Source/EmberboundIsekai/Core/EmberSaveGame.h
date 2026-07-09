#pragma once

#include "CoreMinimal.h"
#include "Data/ResourceTypes.h"
#include "GameFramework/SaveGame.h"
#include "EmberSaveGame.generated.h"

USTRUCT(BlueprintType)
struct EMBERBOUNDISEKAI_API FMonsterAssignmentSave
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, SaveGame)
	FName MonsterId;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	FName AssignmentId;
};

UCLASS()
class EMBERBOUNDISEKAI_API UEmberSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, SaveGame)
	int32 SaveVersion = 1;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	TMap<EEmberResource, int32> Resources;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	TMap<FName, int32> BuildingLevels;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	TMap<FName, int32> MonsterShards;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	TArray<FName> OwnedMonsters;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	TArray<FMonsterAssignmentSave> Assignments;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	TArray<FName> UnlockedTitles;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	TMap<FName, int32> TitleCounters;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	float MasterVolume = 1.0f;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool bShowJoystick = false;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool bSeenTutorial = false;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	TMap<FName, bool> UnlockedFlags;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	TMap<FName, FString> Settings;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	FDateTime LastSaveUtc;
};

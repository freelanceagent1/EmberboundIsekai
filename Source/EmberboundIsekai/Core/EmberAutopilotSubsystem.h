#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TimerManager.h"
#include "EmberAutopilotSubsystem.generated.h"

class ACampGameMode;
class AExpeditionGameMode;

UENUM()
enum class EEmberAutopilotPhase : uint8
{
	Inactive,
	CampStart,
	TravelingToExpedition,
	Expedition,
	TravelingToCamp,
	CampReturn,
	Complete
};

UCLASS()
class EMBERBOUNDISEKAI_API UEmberAutopilotSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	static TArray<FString> GetExpectedMarkerChain();

	void OnCampBeginPlay(ACampGameMode* CampMode);
	void OnExpeditionBeginPlay(AExpeditionGameMode* ExpeditionMode);

private:
	EEmberAutopilotPhase Phase = EEmberAutopilotPhase::Inactive;
	IConsoleObject* AutopilotCommand = nullptr;
	FTimerHandle CampStartTimerHandle;
	FTimerHandle ExpeditionTickTimerHandle;
	FTimerHandle CampReturnTickTimerHandle;
	int32 LastLoggedRoom = 0;
	bool bLoggedRoomsCleared = false;
	bool bLoggedBossKilled = false;
	bool bDefenseStarted = false;
	bool bDefenseWasActive = false;
	bool bOfflineDone = false;

	void AutopilotConsole(const TArray<FString>& Args);
	void StartAutopilot();
	void ResumeForCurrentWorld();
	void BeginCampStart(ACampGameMode* CampMode);
	void TravelToExpedition();
	void BeginExpedition(AExpeditionGameMode* ExpeditionMode);
	void ExpeditionTick();
	void BeginCampReturn(ACampGameMode* CampMode);
	void CampReturnTick();
	void CompleteAutopilot();
	void ForceClearExpeditionEnemies(UWorld* World) const;
	void DamageActiveBoss(AExpeditionGameMode* ExpeditionMode) const;
	bool BuyBuilding(FName BuildingId) const;
};

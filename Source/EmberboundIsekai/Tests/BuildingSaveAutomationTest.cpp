#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Core/EconomySubsystem.h"
#include "Core/SaveManagerSubsystem.h"
#include "Data/BuildingData.h"
#include "Engine/GameInstance.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBuildingUpgradeCostTest, "Emberbound.M4.BuildingUpgradeCost", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBuildingUpgradeCostTest::RunTest(const FString& Parameters)
{
	UBuildingData* Campfire = NewObject<UBuildingData>();
	Campfire->Id = TEXT("campfire-core");
	Campfire->MaxLevel = 5;
	FResourceCost LevelTwoCost;
	LevelTwoCost.Amounts.Add(EEmberResource::Gold, 20);
	LevelTwoCost.Amounts.Add(EEmberResource::Wood, 5);
	Campfire->UpgradeCosts.Add(LevelTwoCost);

	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UEconomySubsystem* Economy = NewObject<UEconomySubsystem>(GameInstance);
	TestNotNull(TEXT("Economy subsystem created with valid GameInstance outer"), Economy);
	if (!Economy)
	{
		return false;
	}
	Economy->Set(EEmberResource::Gold, 20);
	Economy->Set(EEmberResource::Wood, 5);

	TestTrue(TEXT("Level 2 upgrade is affordable with exact resources"), USaveManagerSubsystem::CanAffordUpgrade(Campfire, 1, Economy));
	TestTrue(TEXT("SpendUpgradeCost succeeds with exact resources"), USaveManagerSubsystem::SpendUpgradeCost(Campfire, 1, Economy));
	TestEqual(TEXT("Gold spent to zero"), Economy->Get(EEmberResource::Gold), 0);
	TestEqual(TEXT("Wood spent to zero"), Economy->Get(EEmberResource::Wood), 0);

	return true;
}

#endif

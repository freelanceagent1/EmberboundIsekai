#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Camp/DefenseEventManager.h"
#include "Data/ResourceTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDefenseRewardCalculationTest, "Emberbound.M7.DefenseRewardCalculation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDefenseRewardCalculationTest::RunTest(const FString& Parameters)
{
	const FDefenseRewardAmounts Victory = UDefenseEventManager::CalculateRewards(true, 2);
	TestEqual(TEXT("Victory gold includes watchtower bonus"), Victory.Amounts.FindRef(EEmberResource::Gold), 18);
	TestEqual(TEXT("Victory wood includes watchtower bonus"), Victory.Amounts.FindRef(EEmberResource::Wood), 7);
	TestEqual(TEXT("Victory soul ash includes watchtower bonus"), Victory.Amounts.FindRef(EEmberResource::SoulAsh), 4);
	TestEqual(TEXT("Victory monster essence includes watchtower bonus"), Victory.Amounts.FindRef(EEmberResource::MonsterEssence), 3);

	const FDefenseRewardAmounts Defeat = UDefenseEventManager::CalculateRewards(false, 2);
	TestEqual(TEXT("Defeat gold is 40 percent rounded down after watchtower bonus"), Defeat.Amounts.FindRef(EEmberResource::Gold), 7);
	TestEqual(TEXT("Defeat wood is 40 percent rounded down after watchtower bonus"), Defeat.Amounts.FindRef(EEmberResource::Wood), 2);
	TestEqual(TEXT("Defeat soul ash is 40 percent rounded down after watchtower bonus"), Defeat.Amounts.FindRef(EEmberResource::SoulAsh), 1);
	TestEqual(TEXT("Defeat essence is 40 percent rounded down after watchtower bonus"), Defeat.Amounts.FindRef(EEmberResource::MonsterEssence), 1);

	return true;
}

#endif

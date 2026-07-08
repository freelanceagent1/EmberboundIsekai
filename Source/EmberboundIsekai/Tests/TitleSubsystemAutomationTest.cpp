#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Core/TitleSubsystem.h"
#include "Data/TitleData.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTitlePassiveMathTest, "Emberbound.M8.TitlePassiveMath", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTitlePassiveMathTest::RunTest(const FString& Parameters)
{
	TArray<FTitlePassiveSpec> Passives;
	Passives.Add({ ETitlePassiveEffectType::OfflineProductionPercent, 0.05f });
	Passives.Add({ ETitlePassiveEffectType::UpgradeCostPercent, -0.05f });
	Passives.Add({ ETitlePassiveEffectType::XpGainPercent, 0.05f });
	Passives.Add({ ETitlePassiveEffectType::LowHealthDamagePercent, 0.10f });

	TestEqual(TEXT("Offline title rounds 120 gold up by five percent"), UTitleSubsystem::ApplyOfflineProductionPassive(120, Passives), 126);
	TestEqual(TEXT("Spreadsheet title discounts upgrade costs by five percent"), UTitleSubsystem::ApplyUpgradeCostPassive(20, Passives), 19);
	TestEqual(TEXT("Genre-savvy title rounds 20 XP up by five percent"), UTitleSubsystem::ApplyXpPassive(20, Passives), 21);
	TestEqual(TEXT("Failed chosen one only affects low HP damage"), UTitleSubsystem::ApplyDamagePassive(12.0f, 0.29f, Passives), 13.2f);
	TestEqual(TEXT("Low-health title does not affect healthy player damage"), UTitleSubsystem::ApplyDamagePassive(12.0f, 0.30f, Passives), 12.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTitleUnlockRuleTest, "Emberbound.M8.TitleUnlockRules", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTitleUnlockRuleTest::RunTest(const FString& Parameters)
{
	UTitleData* Title = NewObject<UTitleData>();
	Title->Id = TEXT("spreadsheet-hero");
	Title->UnlockCondition = ETitleUnlockCondition::BuildingUpgrades;
	Title->UnlockParam = 5;

	TestFalse(TEXT("Four building upgrades do not unlock spreadsheet hero"), UTitleSubsystem::DoesCounterSatisfyUnlock(*Title, 4));
	TestTrue(TEXT("Five building upgrades unlock spreadsheet hero"), UTitleSubsystem::DoesCounterSatisfyUnlock(*Title, 5));

	return true;
}

#endif

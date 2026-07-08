#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Core/EmberAutopilotSubsystem.h"
#include "Combat/EmberDamageTextSubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FM10DamageTextPolicyTest, "Emberbound.M10.DamageTextPolicy", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FM10DamageTextPolicyTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Damage text pool is capped for mobile"), UEmberDamageTextSubsystem::GetMaxConcurrentTexts(), 32);
	TestFalse(TEXT("Ordinary hits do not use crit styling"), UEmberDamageTextSubsystem::IsCriticalDamage(24.0f));
	TestTrue(TEXT("Large hits use crit styling"), UEmberDamageTextSubsystem::IsCriticalDamage(25.0f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FM10AutopilotMarkerChainTest, "Emberbound.M10.AutopilotMarkerChain", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FM10AutopilotMarkerChainTest::RunTest(const FString& Parameters)
{
	const TArray<FString> Expected = UEmberAutopilotSubsystem::GetExpectedMarkerChain();
	TestEqual(TEXT("Autopilot marker count"), Expected.Num(), 12);
	TestEqual(TEXT("Starts in camp"), Expected[0], TEXT("EMBER: AUTOPILOT phase=CampStart"));
	TestEqual(TEXT("Travels to expedition"), Expected[1], TEXT("EMBER: AUTOPILOT travel=Expedition"));
	TestEqual(TEXT("Resumes in expedition"), Expected[2], TEXT("EMBER: AUTOPILOT phase=Expedition"));
	TestEqual(TEXT("Clears rooms"), Expected[3], TEXT("EMBER: AUTOPILOT roomsCleared"));
	TestEqual(TEXT("Kills boss"), Expected[4], TEXT("EMBER: AUTOPILOT bossKilled"));
	TestEqual(TEXT("Travels to camp"), Expected[5], TEXT("EMBER: AUTOPILOT travel=Camp"));
	TestEqual(TEXT("Resumes in camp return"), Expected[6], TEXT("EMBER: AUTOPILOT phase=CampReturn"));
	TestEqual(TEXT("Buys campfire"), Expected[7], TEXT("EMBER: AUTOPILOT bought=campfire-core"));
	TestEqual(TEXT("Buys forge"), Expected[8], TEXT("EMBER: AUTOPILOT bought=grim-forge"));
	TestEqual(TEXT("Completes defense"), Expected[9], TEXT("EMBER: AUTOPILOT defenseDone"));
	TestEqual(TEXT("Applies offline rewards"), Expected[10], TEXT("EMBER: AUTOPILOT offlineDone"));
	TestEqual(TEXT("Saves before completion"), Expected[11], TEXT("EMBER: AUTOPILOT saved"));
	return true;
}

#endif

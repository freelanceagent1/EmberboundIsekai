#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Camp/CampBuildingActor.h"
#include "Math/UnrealMathUtility.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FM16CampPresentationDefaultsTest, "Emberbound.M16.CampPresentationDefaults", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FM16CampPresentationDefaultsTest::RunTest(const FString& Parameters)
{
	const TArray<FCampBuildingLayoutSpec> Layout = ACampBuildingActor::CampLayoutSpecs();
	TestEqual(TEXT("camp layout includes all five buildings"), Layout.Num(), 5);

	TSet<FName> SeenIds;
	for (const FCampBuildingLayoutSpec& Spec : Layout)
	{
		TestFalse(TEXT("building id is unique"), SeenIds.Contains(Spec.BuildingId));
		SeenIds.Add(Spec.BuildingId);
		TestNotEqual(TEXT("building display names are not Text placeholders"), ACampBuildingActor::DisplayNameForBuildingId(Spec.BuildingId).ToString(), FString(TEXT("Text")));
	}

	for (int32 Index = 0; Index < Layout.Num(); ++Index)
	{
		for (int32 OtherIndex = Index + 1; OtherIndex < Layout.Num(); ++OtherIndex)
		{
			const float Distance = FVector::Dist2D(Layout[Index].Location, Layout[OtherIndex].Location);
			TestTrue(TEXT("camp buildings have clear spacing at M16 ortho width"), Distance >= 330.0f);
		}
	}

	TestEqual(TEXT("campfire label is real display name"), ACampBuildingActor::DisplayNameForBuildingId(TEXT("campfire-core")).ToString(), FString(TEXT("Campfire Core")));
	TestEqual(TEXT("portal label is full display name"), ACampBuildingActor::DisplayNameForBuildingId(TEXT("expedition-portal")).ToString(), FString(TEXT("Expedition Portal")));

	return true;
}

#endif

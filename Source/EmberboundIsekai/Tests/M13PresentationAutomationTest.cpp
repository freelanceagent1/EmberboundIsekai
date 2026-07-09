#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Camera/CameraComponent.h"
#include "Combat/EmberPlayerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FM13PhonePresentationDefaultsTest, "Emberbound.M13.PhonePresentationDefaults", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FM13PhonePresentationDefaultsTest::RunTest(const FString& Parameters)
{
	const AEmberPlayerCharacter* DefaultPlayer = AEmberPlayerCharacter::StaticClass()->GetDefaultObject<AEmberPlayerCharacter>();
	TestNotNull(TEXT("player default object exists"), DefaultPlayer);
	if (!DefaultPlayer)
	{
		return false;
	}

	const UCameraComponent* Camera = DefaultPlayer->FindComponentByClass<UCameraComponent>();
	TestNotNull(TEXT("player owns gameplay camera"), Camera);
	if (Camera)
	{
		TestEqual(TEXT("gameplay camera uses orthographic projection"), Camera->ProjectionMode, ECameraProjectionMode::Orthographic);
		TestTrue(TEXT("orthographic width frames a landscape phone view"), Camera->OrthoWidth >= 1600.0f && Camera->OrthoWidth <= 2200.0f);
		TestEqual(TEXT("gameplay camera is straight top-down"), Camera->GetRelativeRotation(), FRotator(-90.0f, 0.0f, 0.0f));
		TestTrue(TEXT("gameplay camera is high enough above the 2D board"), Camera->GetRelativeLocation().Z >= 1800.0f);
	}

	const USpringArmComponent* SpringArm = DefaultPlayer->FindComponentByClass<USpringArmComponent>();
	TestNull(TEXT("2D phone presentation does not use a perspective spring arm"), SpringArm);

	TArray<UStaticMeshComponent*> MeshComponents;
	DefaultPlayer->GetComponents<UStaticMeshComponent>(MeshComponents);
	const UStaticMeshComponent* Billboard = nullptr;
	for (const UStaticMeshComponent* Mesh : MeshComponents)
	{
		if (Mesh && Mesh->GetName().Contains(TEXT("BillboardVisual")))
		{
			Billboard = Mesh;
			break;
		}
	}
	TestNotNull(TEXT("player still renders as a flat sprite plane"), Billboard);
	if (Billboard)
	{
		TestEqual(TEXT("sprite plane lies flat in XY for top-down orthographic capture"), Billboard->GetRelativeRotation(), FRotator::ZeroRotator);
	}

	return true;
}

#endif

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Generated/EmberGeneratedArt.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGeneratedArtPathTest, "Emberbound.M5b.GeneratedArtPaths", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGeneratedArtPathTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("green slime resolves to generated character texture"), EmberGeneratedArt::EnemyTexturePath(TEXT("GreenSlime")), FString(TEXT("/Game/Emberbound/Generated/Characters/T_GreenSlime.T_GreenSlime")));
	TestEqual(TEXT("player arrow resolves to generated VFX texture"), EmberGeneratedArt::ProjectileTexturePath(false), FString(TEXT("/Game/Emberbound/Generated/VFX/T_ArrowProjectile.T_ArrowProjectile")));
	TestEqual(TEXT("enemy fireball resolves to generated VFX texture"), EmberGeneratedArt::ProjectileTexturePath(true), FString(TEXT("/Game/Emberbound/Generated/VFX/T_FireProjectile.T_FireProjectile")));
	TestEqual(TEXT("upgrade ids convert from kebab case to Pascal texture names"), EmberGeneratedArt::UpgradeIconTexturePath(TEXT("soul-flame-trail")), FString(TEXT("/Game/Emberbound/Generated/UpgradeIcons/T_SoulFlameTrail.T_SoulFlameTrail")));
	TestEqual(TEXT("HUD icon list covers expected resources"), EmberGeneratedArt::HudIconIds().Num(), 6);
	return true;
}

#endif

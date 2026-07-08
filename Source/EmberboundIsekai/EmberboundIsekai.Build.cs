using UnrealBuildTool;

public class EmberboundIsekai : ModuleRules
{
	public EmberboundIsekai(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.NoPCHs;

		PublicIncludePaths.AddRange(new string[]
		{
			"EmberboundIsekai"
		});

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"UMG",
			"Slate",
			"SlateCore",
			"AssetRegistry",
			"AIModule",
			"NavigationSystem",
			"GameplayTags",
			"Niagara"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}

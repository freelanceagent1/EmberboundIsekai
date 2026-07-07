using UnrealBuildTool;
using System.Collections.Generic;

public class EmberboundIsekaiTarget : TargetRules
{
	public EmberboundIsekaiTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("EmberboundIsekai");
	}
}

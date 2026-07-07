using UnrealBuildTool;
using System.Collections.Generic;

public class EmberboundIsekaiEditorTarget : TargetRules
{
	public EmberboundIsekaiEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("EmberboundIsekai");
	}
}

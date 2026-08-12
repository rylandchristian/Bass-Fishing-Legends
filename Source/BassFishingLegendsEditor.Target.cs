using UnrealBuildTool;
using System.Collections.Generic;

public class BassFishingLegendsEditorTarget : TargetRules
{
	public BassFishingLegendsEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("BassFishingLegends");
	}
}

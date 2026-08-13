using UnrealBuildTool;
using System.Collections.Generic;

public class BassFishingLegendsTarget : TargetRules
{
	public BassFishingLegendsTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		CppCompileWarningSettings.ShadowVariableWarningLevel = WarningLevel.Warning;
		ExtraModuleNames.Add("BassFishingLegends");
	}
}

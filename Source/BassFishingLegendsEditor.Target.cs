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

		// This Linux 5.8.1 drop lists these plugins but does not ship their modules.
		DisablePlugins.Add("PythonScriptPlugin");
		DisablePlugins.Add("PlatformCrypto");
	}
}

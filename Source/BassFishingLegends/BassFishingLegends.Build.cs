using UnrealBuildTool;

public class BassFishingLegends : ModuleRules
{
	public BassFishingLegends(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// UE 5.8 Latest treats MSVC C4458 (local hides class member) as an error.
		// Linux clang in this engine drop ignores -Wshadow, so the same code compiled there.
		CppCompileWarningSettings.ShadowVariableWarningLevel = WarningLevel.Warning;

		// Headers live next to the .cpp files (Game/, Fishing/, …), not under Public/.
		PublicIncludePaths.Add(ModuleDirectory);

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
			"CableComponent",
			"DeveloperSettings",
			"AIModule",
			"NetCore",
			"IrisCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});

		// Editor automation loads the authored Lake. The game module still does
		// not depend on the Water plugin — the test walks class names.
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
		}
	}
}

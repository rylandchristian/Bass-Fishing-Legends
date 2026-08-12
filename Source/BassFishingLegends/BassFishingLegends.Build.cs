using UnrealBuildTool;

public class BassFishingLegends : ModuleRules
{
	public BassFishingLegends(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

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
	}
}

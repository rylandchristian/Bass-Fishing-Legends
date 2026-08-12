using UnrealBuildTool;

public class BassFishingLegends : ModuleRules
{
	public BassFishingLegends(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

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
			"AIModule"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});
	}
}

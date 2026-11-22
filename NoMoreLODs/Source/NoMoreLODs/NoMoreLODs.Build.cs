using UnrealBuildTool;

public class NoMoreLODs : ModuleRules
{
	public NoMoreLODs(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"SML"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"Foliage",
			"Landscape"
		});
	}
}

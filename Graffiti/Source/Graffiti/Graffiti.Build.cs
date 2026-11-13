using UnrealBuildTool;

public class Graffiti : ModuleRules
{
	public Graffiti(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", "CoreUObject", "Engine",
			"DummyHeaders",
			"FactoryGame", "SML"
		});
	}
}

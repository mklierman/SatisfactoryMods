using UnrealBuildTool;

public class BlueprintHighlighted : ModuleRules
{
	public BlueprintHighlighted(ReadOnlyTargetRules Target) : base(Target)
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

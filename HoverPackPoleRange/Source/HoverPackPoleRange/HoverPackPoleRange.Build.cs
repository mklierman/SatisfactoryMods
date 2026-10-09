using UnrealBuildTool;

public class HoverPackPoleRange : ModuleRules
{
	public HoverPackPoleRange(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bLegacyPublicIncludePaths = false;
		CppStandard = CppStandardVersion.Cpp20;

		PublicDependencyModuleNames.AddRange(new[] {
			"Json",
			"Projects",
			"NetCore",
			"EnhancedInput",
			"GameplayTags"
		});

		PrivateDependencyModuleNames.AddRange(new[] {
			"RenderCore",
			"EngineSettings"
		});

		PublicDependencyModuleNames.AddRange(new string[] { "FactoryGame" });

		PublicDependencyModuleNames.AddRange(new[] {
			"Core", "CoreUObject",
			"Engine",
			"InputCore",
			"SlateCore", "Slate", "UMG",
		});

		if (Target.Type == TargetRules.TargetType.Editor) {
			PublicDependencyModuleNames.AddRange(new string[] {"OnlineBlueprintSupport", "AnimGraph"});
		}
		PublicDependencyModuleNames.AddRange(new string[] {"FactoryGame", "SML"});
	}
}

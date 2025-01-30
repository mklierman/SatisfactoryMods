using UnrealBuildTool;
using System.IO;
using System;

public class LoadBalancers : ModuleRules
{
    public LoadBalancers(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;

        //SML transitive dependencies
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

        //FactoryGame transitive dependencies
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
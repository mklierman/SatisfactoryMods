using UnrealBuildTool;

public class AccessTransformerEditor : ModuleRules
{
    public AccessTransformerEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;

        PrivateDependencyModuleNames.AddRange(new[] {
            "Core",
            "CoreUObject",
            "Engine",
            "Projects",
            "InputCore",
            "ApplicationCore",
            "Slate",
            "SlateCore",
            "UnrealEd",
            "LevelEditor",
            "ClassViewer",
            "ToolWidgets",
        });
    }
}

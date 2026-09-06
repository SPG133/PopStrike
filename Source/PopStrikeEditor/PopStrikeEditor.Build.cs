using UnrealBuildTool;
public class PopStrikeEditor : ModuleRules
{
    public PopStrikeEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine" });
        PrivateDependencyModuleNames.AddRange(new[] { "UnrealEd", "UMG", "UMGEditor", "BlueprintGraph", "KismetCompiler", "PopStrike" });
    }
}

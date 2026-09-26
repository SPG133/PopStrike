using UnrealBuildTool;

public class PopStrike : ModuleRules
{
    public PopStrike(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "EnhancedInput",
            "InputCore",
            "UMG",
            "OnlineSubsystemUtils"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore",
            "ImageCore",
            "ImageWrapper"
        });

        // Keep module-local headers available to sources under subdirectories such as Tests.
        PrivateIncludePaths.Add(ModuleDirectory);

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicSystemLibraries.Add("Comdlg32.lib");
        }
    }
}

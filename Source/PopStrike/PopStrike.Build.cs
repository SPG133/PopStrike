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
            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",
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

        // Shared lobby headers and match headers remain visible to this module's sources and tests.
        PrivateIncludePaths.Add(ModuleDirectory);
        PrivateIncludePaths.Add(System.IO.Path.Combine(ModuleDirectory, "level3"));

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicSystemLibraries.Add("Comdlg32.lib");
        }
    }
}

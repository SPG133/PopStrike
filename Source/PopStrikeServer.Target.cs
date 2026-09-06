using UnrealBuildTool;

public class PopStrikeServerTarget : TargetRules
{
    public PopStrikeServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Server;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("PopStrike");
    }
}

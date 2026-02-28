using UnrealBuildTool;

public class OnlyTheDeadTarget : TargetRules
{
    public OnlyTheDeadTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V3;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
        ExtraModuleNames.Add("OnlyTheDead");
    }
}

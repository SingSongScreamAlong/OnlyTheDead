using UnrealBuildTool;

public class OnlyTheDeadEditorTarget : TargetRules
{
    public OnlyTheDeadEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V3;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
        ExtraModuleNames.Add("OnlyTheDead");
    }
}

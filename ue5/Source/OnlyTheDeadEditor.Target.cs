using UnrealBuildTool;
using System.Collections.Generic;

public class OnlyTheDeadEditorTarget : TargetRules
{
    public OnlyTheDeadEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;

        ExtraModuleNames.AddRange(new string[]
        {
            "OnlyTheDead",
            "OnlyTheDeadEditor"
        });
    }
}

//// BEGIN NEW CODE
using UnrealBuildTool;
using System.Collections.Generic;

public class OnlyTheDeadTarget : TargetRules
{
    public OnlyTheDeadTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;

        ExtraModuleNames.AddRange(new string[]
        {
            "OnlyTheDead"
        });
    }
}
//// END NEW CODE

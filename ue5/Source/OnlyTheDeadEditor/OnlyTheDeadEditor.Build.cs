using UnrealBuildTool;

public class OnlyTheDeadEditor : ModuleRules
{
    public OnlyTheDeadEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "UnrealEd",
            "OnlyTheDead"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
        });
    }
}

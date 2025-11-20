//// BEGIN NEW CODE
using UnrealBuildTool;

public class OnlyTheDead : ModuleRules
{
    public OnlyTheDead(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "Niagara",
            "GameplayTags",
            "AIModule"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
        });
    }
}
//// END NEW CODE

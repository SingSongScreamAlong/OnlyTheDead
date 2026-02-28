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
            "GeometryCollectionEngine",  // Chaos Destruction
            "PhysicsCore",
            "Chaos",
            "Niagara"                    // VFX system
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore"
        });

        // Enable UE5 Chaos physics destruction
        PublicDefinitions.Add("WITH_CHAOS=1");
    }
}

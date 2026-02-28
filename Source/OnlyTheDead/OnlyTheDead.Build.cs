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
            "GeometryCollectionEngine",  // Chaos Geometry Collections (fracture debris)
            "PhysicsCore",
            "Chaos",
            "Niagara",                   // GPU VFX — soil geysers, smoke, shrapnel
            // UE 5.7: UDynamicMeshComponent replaces UProceduralMeshComponent.
            // Partial vertex uploads mean only the crater footprint is re-sent to GPU
            // per impact, rather than the entire terrain buffer.
            "GeometryFramework",         // UDynamicMeshComponent
            "DynamicMesh",               // FDynamicMesh3, FDynamicMeshAttributeSet
            "GeometryCore",              // FIndex3i, FVector3d geometry primitives
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore",
            // PCG: production-ready in 5.7. Used by generate_test_level.py today;
            // future: replace Python trench layout with runtime PCG graph for
            // per-session battlefield variation.
            "PCG",
        });

        // WITH_CHAOS is always true in UE 5.7 — Chaos is the only physics backend.
        // The explicit define is no longer needed, but kept for any third-party
        // code that guards on it.
        PublicDefinitions.Add("WITH_CHAOS=1");
    }
}

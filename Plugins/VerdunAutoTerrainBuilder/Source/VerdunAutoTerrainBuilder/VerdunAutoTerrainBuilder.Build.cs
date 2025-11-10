// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VerdunAutoTerrainBuilder : ModuleRules
{
	public VerdunAutoTerrainBuilder(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Landscape",
				"LandscapeEditor",
				"Json",
				"JsonUtilities",
				"RawMesh",
				"ProceduralMeshComponent"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"RHI",
				"RenderCore",
				"ImageWrapper",
				"ImageWriteQueue"
			}
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
	}
}

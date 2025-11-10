// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VerdunAutoTerrainBuilderEditor : ModuleRules
{
	public VerdunAutoTerrainBuilderEditor(ReadOnlyTargetRules Target) : base(Target)
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
				"UnrealEd",
				"VerdunAutoTerrainBuilder",
				"Landscape",
				"LandscapeEditor",
				"EditorStyle",
				"Slate",
				"SlateCore",
				"Json",
				"JsonUtilities",
				"ToolMenus",
				"EditorSubsystem"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"InputCore",
				"PropertyEditor",
				"ContentBrowser",
				"AssetTools",
				"ImageWrapper",
				"RawMesh"
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

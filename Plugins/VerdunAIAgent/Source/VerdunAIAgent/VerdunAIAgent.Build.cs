// Copyright Only The Dead Development Team. All Rights Reserved.

using UnrealBuildTool;

public class VerdunAIAgent : ModuleRules
{
	public VerdunAIAgent(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				// Public include paths
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
				// Private include paths
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"Json",
				"JsonUtilities",
				"HTTP",
				"VerdunAIAgentRuntime"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealEd",
				"EditorScriptingUtilities",
				"Slate",
				"SlateCore",
				"LevelEditor",
				"Projects",
				"PropertyEditor",
				"AssetTools",
				"EditorStyle",
				"ContentBrowser",
				"WorkspaceMenuStructure",
				"ToolMenus",
				"GeometryScripting",
				"GeometryCore",
				"GeometryFramework",
				"DynamicMesh",
				"MeshDescription",
				"StaticMeshDescription",
				"Landscape",
				"LandscapeEditor",
				"Foliage",
				"EditorSubsystem"
			}
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// Dynamically loaded modules
			}
		);
	}
}

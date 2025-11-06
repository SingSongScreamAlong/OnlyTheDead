// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ClaudeTerminalEditor : ModuleRules
{
	public ClaudeTerminalEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"ClaudeTerminal"
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore",
				"UnrealEd",
				"WorkspaceMenuStructure",
				"EditorStyle",
				"ToolMenus",
				"Projects",
				"InputCore",
				"LevelEditor",
				"PropertyEditor",
				"Landscape",
				"LandscapeEditor",
				"ImageWrapper"
			}
		);
	}
}

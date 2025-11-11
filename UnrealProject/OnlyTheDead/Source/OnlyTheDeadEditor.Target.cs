// Copyright Epic Games, Inc. All Rights Reserved.
// Only The Dead - WWI Survival Game - Battle of Verdun 1916

using UnrealBuildTool;
using System.Collections.Generic;

public class OnlyTheDeadEditorTarget : TargetRules
{
	public OnlyTheDeadEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("OnlyTheDead");
	}
}

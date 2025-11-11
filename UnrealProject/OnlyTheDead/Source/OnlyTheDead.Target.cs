// Copyright Epic Games, Inc. All Rights Reserved.
// Only The Dead - WWI Survival Game - Battle of Verdun 1916

using UnrealBuildTool;
using System.Collections.Generic;

public class OnlyTheDeadTarget : TargetRules
{
	public OnlyTheDeadTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("OnlyTheDead");
	}
}

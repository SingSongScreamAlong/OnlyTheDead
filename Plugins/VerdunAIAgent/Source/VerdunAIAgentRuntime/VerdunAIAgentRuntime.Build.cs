// Copyright Only The Dead Development Team. All Rights Reserved.

using UnrealBuildTool;

public class VerdunAIAgentRuntime : ModuleRules
{
	public VerdunAIAgentRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Json",
				"JsonUtilities",
				"HTTP"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// Private dependencies
			}
		);
	}
}

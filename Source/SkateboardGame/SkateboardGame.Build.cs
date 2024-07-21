// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SkateboardGame : ModuleRules
{
	public SkateboardGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}

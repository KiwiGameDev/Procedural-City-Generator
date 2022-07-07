// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class IECAPSTONE : ModuleRules
{
	public IECAPSTONE(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Landscape", "ProceduralMeshComponent", "CableComponent"});

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "StreetMapRuntime", "CableComponent" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true

		OptimizeCode = CodeOptimization.Never;
	}
}

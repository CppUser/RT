// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RT : ModuleRules
{
	public RT(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				"RT"
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreOnline",
				"CoreUObject",
				"ApplicationCore",
				"Engine",
				"PhysicsCore",
				"GameplayTags",
				"GameplayTasks",
				"GameplayAbilities",
				"AIModule",
				"ModularGameplay",
				"GameFeatures",
				"Niagara",
				"ControlFlows",
				"PropertyPath"
			}
		);
		
		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"InputCore",
				"Slate",
				"SlateCore",
				"RenderCore",
				"DeveloperSettings",
				"EnhancedInput",
				"Projects",
				"UMG",
				"CommonUI",
				"CommonInput",
				"AudioMixer",
				"AudioModulation",
				"EngineSettings",
				"Json",
			}
		);
	}
}

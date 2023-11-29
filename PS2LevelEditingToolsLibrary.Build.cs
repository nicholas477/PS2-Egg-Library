// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class PS2LevelEditingToolsLibrary : ModuleRules
{
	public PS2LevelEditingToolsLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		CppStandard = CppStandardVersion.Cpp20;
		Type = ModuleType.External;
		PublicSystemIncludePaths.Add("$(ModuleDir)/PS2-Egg-Library/include");

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Add the import library
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "PS2-Egg-Library", "build", "Debug", "ps2-egg.lib"));
		}
		// else if (Target.Platform == UnrealTargetPlatform.Mac)
		// {
			// PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "Mac", "Release", "libExampleLibrary.dylib"));
			// RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/PS2LevelEditingToolsLibrary/Mac/Release/libExampleLibrary.dylib");
		// }
		// else if (Target.Platform == UnrealTargetPlatform.Linux)
		// {
			// string ExampleSoPath = Path.Combine("$(PluginDir)", "Binaries", "ThirdParty", "PS2LevelEditingToolsLibrary", "Linux", "x86_64-unknown-linux-gnu", "libExampleLibrary.so");
			// PublicAdditionalLibraries.Add(ExampleSoPath);
			// PublicDelayLoadDLLs.Add(ExampleSoPath);
			// RuntimeDependencies.Add(ExampleSoPath);
		// }
	}
}

// Copyright 2017 Mike Fricker. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
  public class StreetMapRuntime : ModuleRules
  {
    public StreetMapRuntime(ReadOnlyTargetRules Target)
    : base(Target)
    {
      PrivateDependencyModuleNames.AddRange(
        new string[] {
          "Core",
          "CoreUObject",
          "Engine",
          "RHI",
          "RenderCore"
        }
      );

      if (Target.Type == TargetType.Editor)
      {
          PrivateDependencyModuleNames.Add("PropertyEditor");
      }

      PrivateIncludePaths.AddRange(new string[]{"StreetMapRuntime/Private"});
    }
  }
}

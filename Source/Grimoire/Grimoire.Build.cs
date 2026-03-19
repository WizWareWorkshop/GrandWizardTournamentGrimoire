using UnrealBuildTool;

public class Grimoire : ModuleRules
{
    public Grimoire(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "GameplayTags"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new[]
            {
                "Slate",
                "SlateCore"
            }
        );
    }
}
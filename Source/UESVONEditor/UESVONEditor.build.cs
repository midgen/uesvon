using UnrealBuildTool;

public class UESVONEditor : ModuleRules
{
	public UESVONEditor(ReadOnlyTargetRules Target) : base(Target)
    {

    PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine",  "GOAPer", "InputCore"});

    PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "PropertyEditor", "EditorStyle", "UnrealEd", "GraphEditor", "BlueprintGraph" });
	
	PrivateIncludePaths.AddRange(new string[] { "UESVONEditor/Private",	} );
	
	PrivateIncludePaths.AddRange(new string[] { "UESVONEditor/Public/Shared", } );

    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

    }
};
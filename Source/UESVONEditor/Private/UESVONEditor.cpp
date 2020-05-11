#include "UESVONEditor/UESVONEditor.h"
#include "UESVONEditor/Private/SVONVolumeDetails.h"

#include <Editor/PropertyEditor/Public/PropertyEditorModule.h>

IMPLEMENT_GAME_MODULE(FUESVONEditorModule, UESVONEditor);

DEFINE_LOG_CATEGORY(UESVONEditor)

#define LOCTEXT_NAMESPACE "UESVONEditor"

void FUESVONEditorModule::StartupModule()
{
	UE_LOG(UESVONEditor, Warning, TEXT("UESVONEditorModule: Log Started"));

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.RegisterCustomClassLayout("SVONVolume", FOnGetDetailCustomizationInstance::CreateStatic(&FSVONVolumeDetails::MakeInstance));
}

void FUESVONEditorModule::ShutdownModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	UE_LOG(UESVONEditor, Warning, TEXT("UESVONEditorModule: Log Ended"));
}

#undef LOCTEXT_NAMESPACE
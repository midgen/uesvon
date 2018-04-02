#include "UESVONEditor.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"

IMPLEMENT_GAME_MODULE(FUESVONEditorModule, UESVONEditor);

DEFINE_LOG_CATEGORY(UESVONEditor)

#define LOCTEXT_NAMESPACE "UESVONEditor"

void FUESVONEditorModule::StartupModule()
{
	UE_LOG(UESVONEditor, Warning, TEXT("UESVONEditorModule: Log Started"));

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	//Custom properties
	//PropertyModule.RegisterCustomPropertyTypeLayout("GOAPAtom", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGOAPStateCustomization::MakeInstance));

	//create your factory and shared pointer to it.
	//TSharedPtr<FGOAPAtomPinFactory> GOAPAtomPinFactory = MakeShareable(new FGOAPAtomPinFactory());
	//and now register it.
	//FEdGraphUtilities::RegisterVisualPinFactory(GOAPAtomPinFactory);
}

void FUESVONEditorModule::ShutdownModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	//Custom properties
	//PropertyModule.UnregisterCustomPropertyTypeLayout("GOAPAtom");
	UE_LOG(UESVONEditor, Warning, TEXT("UESVONEditorModule: Log Ended"));
}

#undef LOCTEXT_NAMESPACE
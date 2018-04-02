#pragma once

#include "Engine.h"
#include "ModuleManager.h"
#include "UnrealEd.h"


DECLARE_LOG_CATEGORY_EXTERN(UESVONEditor, All, All)

class FUESVONEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

};
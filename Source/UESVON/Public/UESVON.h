#pragma once

#include <Runtime/Core/Public/Modules/ModuleManager.h>

#if WITH_EDITOR
DECLARE_LOG_CATEGORY_EXTERN(UESVON, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(VUESVON, Log, All);
#endif

class FUESVONModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
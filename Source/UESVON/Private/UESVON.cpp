// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "UESVON.h"

#if WITH_EDITOR

DEFINE_LOG_CATEGORY(UESVON);
DEFINE_LOG_CATEGORY(VUESVON);

#endif

#define LOCTEXT_NAMESPACE "FUESVONModule"

void FUESVONModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FUESVONModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUESVONModule, UESVON)
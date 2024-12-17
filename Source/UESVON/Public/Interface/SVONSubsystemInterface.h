#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "SVONSubsystemInterface.generated.h"

class ASVONVolume;
class USVONNavigationComponent;

UINTERFACE(MinimalAPI)
class USVONSubsystemInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *  Interface for interacting with the main navigation subsystem
 */
class UESVON_API ISVONSubsystemInterface
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual void RegisterVolume(const ASVONVolume* Volume) = 0;
	UFUNCTION()
	virtual void UnRegisterVolume(const ASVONVolume* Volume) = 0;
	UFUNCTION()
	virtual void RegisterNavComponent(USVONNavigationComponent* NavComponent) = 0;
	UFUNCTION()
	virtual void UnRegisterNavComponent(USVONNavigationComponent* NavComponent) = 0;
	UFUNCTION()
	virtual const ASVONVolume* GetVolumeForPosition(const FVector& Position) = 0;
	
};

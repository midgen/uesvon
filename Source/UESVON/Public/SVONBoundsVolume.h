// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "SVONBoundsVolume.generated.h"


/**
 * 
 */
UCLASS()
class UESVON_API ASVONBoundsVolume : public AVolume
{
	GENERATED_UCLASS_BODY()
	
public:

	//~ Begin AActor Interface
	virtual void PostRegisterAllComponents() override;
	virtual void PostUnregisterAllComponents() override;
	//~ End AActor Interface
#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditUndo() override;
	//~ End UObject Interface
#endif // WITH_EDITOR
	
	
};

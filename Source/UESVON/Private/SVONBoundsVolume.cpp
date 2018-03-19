// Fill out your copyright notice in the Description page of Project Settings.

#include "SVONBoundsVolume.h"
#include "Engine/CollisionProfile.h"
#include "Components/BrushComponent.h"



ASVONBoundsVolume::ASVONBoundsVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GetBrushComponent()->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	GetBrushComponent()->Mobility = EComponentMobility::Static;

	BrushColor = FColor(255, 255, 255, 255);

	bColored = true;
}

#if WITH_EDITOR

void ASVONBoundsVolume::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void ASVONBoundsVolume::PostEditUndo()
{
	Super::PostEditUndo();
}

#endif // WITH_EDITOR

void ASVONBoundsVolume::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();
}

void ASVONBoundsVolume::PostUnregisterAllComponents()
{
	Super::PostUnregisterAllComponents();
}
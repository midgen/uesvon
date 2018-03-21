#pragma once

#include "CoreMinimal.h"
#include "SVONLink.h"

struct UESVON_API SVONNode
{
	FVector myPosition;
	
	SVONLink myParent;
	SVONLink myFirstChild;

	SVONLink myNeighbours[6];

	SVONNode(uint8 aLayer) : 
		myPosition(FVector(0.f)),
		myParent(SVONLink(aLayer, 0, 0)),
		myFirstChild(SVONLink(aLayer, 0, 0)) {}

	SVONNode() :
		myPosition(FVector(0.f)),
		myParent(SVONLink(0, 0, 0)),
		myFirstChild(SVONLink(0, 0, 0)) {}
};
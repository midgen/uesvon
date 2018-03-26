#pragma once

#include "CoreMinimal.h"
#include "SVONLink.h"
#include "SVONDefines.h"

struct UESVON_API SVONNode
{
	FVector myPosition;
	mortoncode myCode;

	nodeindex myParentIndex;
	nodeindex myFirstChildIndex;

	SVONLink myNeighbours[6];

	SVONNode(uint8 aLayer) :
		myPosition(FVector(0.f)),
		myParentIndex(-1),
		myFirstChildIndex(-1) {}

	SVONNode() :
		myPosition(FVector(0.f)),
		myParentIndex(-1),
		myFirstChildIndex(-1) {}

};
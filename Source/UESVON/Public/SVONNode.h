#pragma once

#include "CoreMinimal.h"
#include "SVONLink.h"
#include "SVONDefines.h"

struct UESVON_API SVONNode
{
	//FVector myPosition;
	mortoncode myCode;

	nodeindex myParentIndex;
	nodeindex myFirstChildIndex;

	SVONLink myNeighbours[6];

	SVONNode(uint8 aLayer) :
		myParentIndex(-1),
		myFirstChildIndex(-1) {}

	SVONNode() :
		myParentIndex(-1),
		myFirstChildIndex(-1) {}

};
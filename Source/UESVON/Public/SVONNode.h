#pragma once

#include "CoreMinimal.h"
#include "SVONLink.h"
#include "SVONDefines.h"

struct UESVON_API SVONNode
{
	//FVector myPosition;
	mortoncode_t myCode;

	nodeindex_t myParentIndex;
	SVONLink myFirstChildIndex;

	SVONLink myNeighbours[6];

	SVONNode(uint8 aLayer) :
		myParentIndex(-1),
		myFirstChildIndex(SVONLink::GetInvalidLink()) {}

	SVONNode() :
		myParentIndex(-1),
		myFirstChildIndex(SVONLink::GetInvalidLink()) {}

};
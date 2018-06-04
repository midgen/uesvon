#pragma once

#include "CoreMinimal.h"
#include "SVONLink.h"
#include "SVONDefines.h"

struct UESVON_API SVONNode
{
	mortoncode_t myCode;

	SVONLink myParent;
	SVONLink myFirstChild;

	SVONLink myNeighbours[6];

	SVONNode(uint8 aLayer) :
		myParent(SVONLink::GetInvalidLink()),
		myFirstChild(SVONLink::GetInvalidLink()) {}

	SVONNode() :
		myParent(SVONLink::GetInvalidLink()),
		myFirstChild(SVONLink::GetInvalidLink()) {}
};
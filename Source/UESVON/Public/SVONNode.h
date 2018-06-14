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

	SVONNode() :
		myParent(SVONLink::GetInvalidLink()),
		myFirstChild(SVONLink::GetInvalidLink()) {}

	bool HasChildren() const { return myFirstChild.IsValid(); }

};
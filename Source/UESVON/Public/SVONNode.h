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
		myCode(0),
		myParent(SVONLink::GetInvalidLink()),
		myFirstChild(SVONLink::GetInvalidLink()) {}

	bool HasChildren() const { return myFirstChild.IsValid(); }

};

FORCEINLINE FArchive &operator <<(FArchive &Ar, SVONNode& aSVONNode)
{
	Ar << aSVONNode.myCode;
	Ar << aSVONNode.myParent;
	Ar << aSVONNode.myFirstChild;

	for (int i = 0; i < 6; i++)
	{
		Ar << aSVONNode.myNeighbours[i];
	}

	return Ar;
}
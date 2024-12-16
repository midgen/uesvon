#pragma once

#include <UESVON/Public/Data/SVONLink.h>
#include <UESVON/Public/Data/SVONDefines.h>

struct UESVON_API SVONNode
{
	mortoncode_t Code;

	SVONLink Parent;
	SVONLink FirstChild;

	SVONLink myNeighbours[6];

	SVONNode() :
		Code(0),
		Parent(SVONLink::GetInvalidLink()),
		FirstChild(SVONLink::GetInvalidLink()) {}

	bool HasChildren() const { return FirstChild.IsValid(); }

};

FORCEINLINE FArchive &operator <<(FArchive &Ar, SVONNode& aSVONNode)
{
	Ar << aSVONNode.Code;
	Ar << aSVONNode.Parent;
	Ar << aSVONNode.FirstChild;

	for (int i = 0; i < 6; i++)
	{
		Ar << aSVONNode.myNeighbours[i];
	}

	return Ar;
}
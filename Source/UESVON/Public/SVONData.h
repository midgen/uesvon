#pragma once

#include "UESVON/Public/SVONLeafNode.h"
#include "UESVON/Public/SVONNode.h"

struct SVONData
{
	// SVO data
	TArray<TArray<SVONNode>> myLayers;
	TArray<SVONLeafNode> myLeafNodes;

	void Reset()
	{
		myLayers.Empty();
		myLeafNodes.Empty();
	}

	int GetSize()
	{
		int result = 0;
		result += myLeafNodes.Num() * sizeof(SVONLeafNode);
		for (int i = 0; i < myLayers.Num(); i++)
		{
			result += myLayers[i].Num() * sizeof(SVONNode);
		}

		return result;
	}
};

FORCEINLINE FArchive& operator<<(FArchive& Ar, SVONData& aSVONData)
{
	Ar << aSVONData.myLayers;
	Ar << aSVONData.myLeafNodes;

	return Ar;
}
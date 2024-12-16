#pragma once

#include <UESVON/Public/Data/SVONLeafNode.h>
#include <UESVON/Public/Data/SVONNode.h>

#include "SVONOctreeData.generated.h"

USTRUCT()
struct FSVONOctreeData
{
	GENERATED_BODY()

	// SVO data
	TArray<TArray<SVONNode>> Layers;
	TArray<SVONLeafNode> LeafNodes;
	// temporary data used during nav data generation first pass rasterize
	TArray<TSet<mortoncode_t>> BlockedIndices;

	void Reset()
	{
		Layers.Empty();
		LeafNodes.Empty();
	}

	int GetSize()
	{
		int Result = 0;
		Result += LeafNodes.Num() * sizeof(SVONLeafNode);
		for (int i = 0; i < Layers.Num(); i++)
		{
			Result += Layers[i].Num() * sizeof(SVONNode);
		}

		return Result;
	}

	uint8 NumLayers = 0;
	int NumBytes = 0;

	const uint8 GetNumLayers() const { return NumLayers; }
	TArray<SVONNode>& GetLayer(layerindex_t aLayer) { return Layers[aLayer]; };
	const TArray<SVONNode>& GetLayer(layerindex_t aLayer) const { return Layers[aLayer]; };
	const SVONNode& GetNode(const SVONLink& aLink) const;
	const SVONLeafNode& GetLeafNode(nodeindex_t aIndex) const;
	void GetLeafNeighbours(const SVONLink& aLink, TArray<SVONLink>& oNeighbours) const;
	void GetNeighbours(const SVONLink& aLink, TArray<SVONLink>& oNeighbours) const;
};

FORCEINLINE FArchive& operator<<(FArchive& Ar, FSVONOctreeData& SVONData)
{
	Ar << SVONData.Layers;
	Ar << SVONData.LeafNodes;
	Ar << SVONData.NumLayers;

	return Ar;
}

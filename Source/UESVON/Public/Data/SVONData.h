#pragma once

#include <UESVON/Public/Data/SVONOctreeData.h>
#include <UESVON/Public/Data/SVONGenerationParameters.h>

#include "SVONData.generated.h"

class ISVONCollisionQueryInterface;
class ISVONDebugDrawInterface;
struct FSVONGenerationParamaters;

USTRUCT()
struct UESVON_API FSVONData
{
	GENERATED_BODY()

	// SVO data
	UPROPERTY()
	FSVONOctreeData OctreeData;

public:
	void SetExtents(const FVector& Origin, const FVector& Extents);
	void SetDebugPosition(const FVector& DebugPosition);

	void ResetForGeneration();
	void UpdateGenerationParameters(const FSVONGenerationParameters& Params);
	const FSVONGenerationParameters& GetParams() const;
	void Generate(UWorld& World, const ISVONCollisionQueryInterface& CollisionInterface, const ISVONDebugDrawInterface& DebugInterface);

	bool GetLinkPosition(const SVONLink& aLink, FVector& oPosition) const;
	bool GetNodePosition(layerindex_t aLayer, mortoncode_t aCode, FVector& oPosition) const;
	float GetVoxelSize(layerindex_t aLayer) const;

	//~ Begin UObject
	//void Serialize(FArchive& Ar) override;
	//~ End UObject 

private:
	FSVONGenerationParameters GenerationParameters;
	int32 GetNumNodesInLayer(layerindex_t aLayer) const;
	int32 GetNumNodesPerSide(layerindex_t aLayer) const;

	bool IsBlocked(const FVector& aPosition, const float aSize) const;
	bool IsInDebugRange(const FVector& aPosition) const;
	bool IsAnyMemberBlocked(layerindex_t aLayer, mortoncode_t aCode) const;
	bool GetIndexForCode(layerindex_t aLayer, mortoncode_t aCode, nodeindex_t& oIndex) const;

	void BuildNeighbourLinks(layerindex_t aLayer);
	bool FindLinkInDirection(layerindex_t aLayer, const nodeindex_t aNodeIndex, uint8 aDir, SVONLink& oLinkToUpdate, FVector& aStartPosForDebug);

	void RasterizeLeafNode(FVector& aOrigin, nodeindex_t aLeafIndex, const ISVONCollisionQueryInterface& CollisionInterface, const ISVONDebugDrawInterface& DebugInterface);
	void RasteriseLayer(layerindex_t aLayer, const ISVONCollisionQueryInterface& CollisionInterface, const ISVONDebugDrawInterface& DebugInterface);

	void FirstPassRasterise(UWorld& World);
};

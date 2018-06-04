

#include "SVONMediator.h"
#include "CoreMinimal.h"
#include "SVONVolume.h"
#include "SVONLink.h"
#include "DrawDebugHelpers.h"

bool SVONMediator::GetLinkFromPosition(const FVector& aPosition, const ASVONVolume& aVolume, SVONLink& oLink)
{
	// Position is outside the volume, no can do
	if (!aVolume.EncompassesPoint(aPosition))
	{
		return false;
	}

	FBox box = aVolume.GetComponentsBoundingBox(true);

	FVector origin;
	FVector extent;

	box.GetCenterAndExtents(origin, extent);
	// The z-order origin of the volume (where code == 0)
	FVector zOrigin = origin - extent;
	// The local position of the point in volume space
	FVector localPos = aPosition - zOrigin;


	int layerIndex = aVolume.GetMyNumLayers() - 1;
	nodeindex_t nodeIndex = 0;
	while (layerIndex >= 0 && layerIndex < aVolume.GetMyNumLayers())
	{
		// Get the layer and voxel size

		const TArray<SVONNode>& layer = aVolume.GetLayer(layerIndex);
		// Calculate the XYZ coordinates

		FIntVector voxel;
		GetVolumeXYZ(aPosition, aVolume, layerIndex, voxel);
		uint_fast32_t x, y, z;
		x = voxel.X;
		y = voxel.Y;
		z = voxel.Z;


		// Get the morton code we want for this layer
		mortoncode_t code = morton3D_64_encode(x, y, z);

		for (nodeindex_t j = nodeIndex; j < layer.Num(); j++)
		{
			// This is the node we are in
			if (layer[j].myCode == code)
			{
				// There are no child nodes, so this is our nav position
				if (!layer[j].myFirstChild.IsValid())// && layerIndex > 0)
				{
					oLink.myLayerIndex = layerIndex;
					oLink.myNodeIndex = j;
					oLink.mySubnodeIndex = 0;
					return true;
				}

				// If this is a leaf node, we need to find our subnode
				if (layerIndex == 0)
				{
					const SVONLeafNode& leaf = aVolume.GetLeafNode(layer[j].myFirstChild.myNodeIndex);
					// We need to calculate the node local position to get the morton code for the leaf
					float voxelSize = aVolume.GetVoxelSize(layerIndex);
					// The world position of the 0 node
					FVector nodePosition;
					aVolume.GetNodePosition(layerIndex, layer[j].myCode, nodePosition);
					// The morton origin of the node
					FVector nodeOrigin = nodePosition - FVector(voxelSize * 0.5f);
					// The requested position, relative to the node origin
					FVector nodeLocalPos = aPosition - nodeOrigin;
					// Now get our voxel coordinates
					FIntVector coord;
					coord.X = FMath::FloorToInt((nodeLocalPos.X / (voxelSize * 0.25f)));
					coord.Y = FMath::FloorToInt((nodeLocalPos.Y / (voxelSize * 0.25f)));
					coord.Z = FMath::FloorToInt((nodeLocalPos.Z / (voxelSize * 0.25f)));

					// So our link is.....*drum roll*
					oLink.myLayerIndex = 0; // Layer 0 (leaf)
					oLink.myNodeIndex = j; // This index
					oLink.mySubnodeIndex = morton3D_64_encode(coord.X, coord.Y, coord.Z); // This morton code is our key into the 64-bit leaf node

					return true;
				}
				
				// If we've got here, the current node has a child, and isn't a leaf, so lets go down...
				layerIndex = layer[j].myFirstChild.GetLayerIndex();
				nodeIndex = layer[j].myFirstChild.GetNodeIndex();

				break; //stop iterating this layer
			}
		}
	}

	return false;
}

void SVONMediator::GetVolumeXYZ(const FVector& aPosition, const ASVONVolume& aVolume, const int aLayer, FIntVector& oXYZ)
{
	FBox box = aVolume.GetComponentsBoundingBox(true);

	FVector origin;
	FVector extent;

	box.GetCenterAndExtents(origin, extent);
	// The z-order origin of the volume (where code == 0)
	FVector zOrigin = origin - extent;
	// The local position of the point in volume space
	FVector localPos = aPosition - zOrigin;

	int layerIndex = aLayer;

	// Get the layer and voxel size
	float voxelSize = aVolume.GetVoxelSize(layerIndex);
	
	// Calculate the XYZ coordinates

	oXYZ.X = FMath::FloorToInt((localPos.X / voxelSize));
	oXYZ.Y = FMath::FloorToInt((localPos.Y / voxelSize));
	oXYZ.Z = FMath::FloorToInt((localPos.Z / voxelSize));

}

bool SVONMediator::FindPath(ASVONVolume& aVolume, const SVONLink& aStart, const SVONLink& aGoal, TMap<SVONLink, SVONLink>& oCameFrom)
{
	TArray<SVONLink> openSet;
	TSet<SVONLink> closedSet;

	TMap<SVONLink, float>    gScore;
	TMap<SVONLink, float>    fScore;

	openSet.Add(aStart);
	oCameFrom[aStart] = aStart;
	gScore[aStart] = 0;
	fScore[aStart] = HeuristicScore(aVolume, aStart, aGoal); // Distance to target

	while (openSet.Num() > 0)
	{
		SVONLink current;
		float lowestScore = FLT_MAX;
		for (SVONLink& link : openSet)
		{
			if (fScore[link] < lowestScore)
				current = link;
		}

		
		openSet.Remove(current);
		closedSet.Add(current);

		if (current == aGoal)
			return true;

		const SVONNode& currentNode = aVolume.GetNode(current);
		for (const SVONLink& neighbour : currentNode.myNeighbours)
		{
			if (neighbour.IsValid())
			{
				if (closedSet.Contains(neighbour))
					continue;

				if (!openSet.Contains(neighbour))
					openSet.Add(neighbour);

				float t_gScore = gScore.Contains(neighbour) ? gScore[neighbour] : FLT_MAX + DistanceBetween(aVolume, current, neighbour);

				if (t_gScore >= (gScore.Contains(neighbour) ? gScore[neighbour] : FLT_MAX))
					continue;

				oCameFrom[neighbour] = current;
				gScore[neighbour] = t_gScore;
				fScore[neighbour] = gScore[neighbour] + HeuristicScore(aVolume, neighbour, aGoal);
			}	
		}
	}

	return false;
}

float SVONMediator::HeuristicScore(const ASVONVolume& aVolume, const SVONLink& aStart, const SVONLink& aTarget)
{
	return DistanceBetween(aVolume, aStart, aTarget);
}

float SVONMediator::DistanceBetween(const ASVONVolume& aVolume, const SVONLink& aStart, const SVONLink& aTarget)
{
	FVector startPos(0.f), endPos(0.f);
	const SVONNode& startNode = aVolume.GetNode(aStart);
	const SVONNode& endNode = aVolume.GetNode(aTarget);
	aVolume.GetNodePosition(aStart.GetLayerIndex(), startNode.myCode, startPos);
	aVolume.GetNodePosition(aTarget.GetLayerIndex(), endNode.myCode, endPos);
	return (startPos - endPos).Size();
}

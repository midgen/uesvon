// Fill out your copyright notice in the Description page of Project Settings.

#include "SVONVolume.h"
#include "Engine/CollisionProfile.h"
#include "Components/BrushComponent.h"
#include "DrawDebugHelpers.h"
#include <chrono>

using namespace std::chrono;

ASVONVolume::ASVONVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GetBrushComponent()->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	GetBrushComponent()->Mobility = EComponentMobility::Static;

	BrushColor = FColor(255, 255, 255, 255);

	bColored = true;
}

#if WITH_EDITOR

void ASVONVolume::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void ASVONVolume::PostEditUndo()
{
	Super::PostEditUndo();
}

void ASVONVolume::OnPostShapeChanged()
{

}

#endif // WITH_EDITOR

/************************************************************************/
/* Regenerates the Sparse Voxel Octree Navmesh                          */
/************************************************************************/
bool ASVONVolume::Generate()
{
	FlushPersistentDebugLines(GetWorld());

	// Get bounds and extent
	FBox bounds = GetComponentsBoundingBox(true);
	bounds.GetCenterAndExtents(myOrigin, myExtent);

	// Setup timing
	milliseconds startMs = duration_cast<milliseconds>(
		system_clock::now().time_since_epoch()
		);

	// Clear data (for now)
	myBlockedIndices.Empty();
	myLayers.Empty();

	myNumLayers = myVoxelPower + 1;

	// Rasterize at Layer 1
	FirstPassRasterize();

	// Allocate the leaf node data
	myLeafNodes.Empty();
	myLeafNodes.AddDefaulted(myBlockedIndices[0].Num() * 8);

	// Add layers
	for (int i = 0; i < myNumLayers; i++)
	{
		myLayers.Emplace();
	}

	// Rasterize layer, bottom up, adding parent/child links
	for (int i = 0; i < myNumLayers; i++)
	{
		RasterizeLayer(i);
	}

	// Now traverse down, adding neighbour links
	for (int i = myNumLayers - 2; i >= 0; i--)
	{
		BuildNeighbourLinks(i);
	}

	int32 buildTime = (duration_cast<milliseconds>(
		system_clock::now().time_since_epoch()
		) - startMs).count();

	int32 totalNodes = 0;

	for (int i = 0; i < myNumLayers; i++)
	{
		totalNodes += myLayers[i].Num();
	}

	int32 totalBytes = sizeof(SVONNode) * totalNodes;
	totalBytes += sizeof(SVONLeafNode) * myLeafNodes.Num();

	UE_LOG(UESVON, Display, TEXT("Generation Time : %d"), buildTime);
	UE_LOG(UESVON, Display, TEXT("Total Layers-Nodes : %d-%d"), myNumLayers, totalNodes);
	UE_LOG(UESVON, Display, TEXT("Total Leaf Nodes : %d"), myLeafNodes.Num());
	UE_LOG(UESVON, Display, TEXT("Total Size (bytes): %d"), totalBytes);


	return true;
}

bool ASVONVolume::FirstPassRasterize()
{
	// Add the first layer of blocking
	myBlockedIndices.Emplace();

	int32 numNodes = GetNodesInLayer(1);
	for (int32 i = 0; i < numNodes; i++)
	{
		FVector position;
		GetNodePosition(1, i, position);

		if (GetWorld()->OverlapBlockingTestByChannel(position, FQuat::Identity, myCollisionChannel, FCollisionShape::MakeBox(FVector(GetVoxelSize(1) * 0.5f))))
		{
			myBlockedIndices[0].Add(i);
		}
	}

	// Do we need to do this? Should be in order anyway
	/*myBlockedIndices.Sort([](const uint_fast64_t& A, const uint_fast64_t& B) {
		return B > A;
	});
*/
	int layerIndex = 0;

	while (myBlockedIndices[layerIndex].Num() > 1)
	{
		// Add a new layer to structure
		myBlockedIndices.Emplace();
		// Add any parent morton codes to the new layer
		for (mortoncode& code : myBlockedIndices[layerIndex])
		{
			myBlockedIndices[layerIndex + 1].Add(code >> 3);
		}
		layerIndex++;
	}

	return true;
}

bool ASVONVolume::GetNodePosition(layerindex aLayer, mortoncode aCode, FVector& oPosition)
{
	float voxelSize = GetVoxelSize(aLayer);
	uint_fast32_t x, y, z;
	morton3D_64_decode(aCode, x, y, z);
	oPosition = myOrigin - myExtent + FVector(x * voxelSize, y * voxelSize, z * voxelSize) + FVector(voxelSize * 0.5f);
	return true;
}



float ASVONVolume::GetVoxelSize(layerindex aLayer)
{
	return (myExtent.X / FMath::Pow(2, myVoxelPower)) * (FMath::Pow(2.0f, aLayer + 1.0f));
}


int32 ASVONVolume::GetNodesInLayer(layerindex aLayer)
{
	return FMath::Pow(FMath::Pow(2, (myVoxelPower - (aLayer))), 3);
}

int32 ASVONVolume::GetNodesPerSide(layerindex aLayer)
{
	return FMath::Pow(2, (myVoxelPower - (aLayer)));
}

void ASVONVolume::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();
}

void ASVONVolume::PostUnregisterAllComponents()
{
	Super::PostUnregisterAllComponents();
}

void ASVONVolume::BuildNeighbourLinks(layerindex aLayer)
{

	TArray<SVONNode>& layer = GetLayer(aLayer);
	layerindex searchLayer = aLayer;

	// For each node
	for (nodeindex i = 0; i < layer.Num(); i++)
	{
		SVONNode& node = layer[i];
		// Get our world co-ordinate
		uint_fast32_t x, y, z;
		morton3D_64_decode(node.myCode, x, y, z);
		nodeindex backtrackIndex = -1;
		nodeindex index = i;
		FVector nodePos;
		GetNodePosition(aLayer, node.myCode, nodePos);

		// For each direction
		for (int d = 0; d < 6; d++)
		{
			SVONLink& linkToUpdate = node.myNeighbours[d];

			backtrackIndex = index;

			while (!FindLinkInDirection(searchLayer, index, d, linkToUpdate, nodePos)
				&& aLayer < myLayers.Num() - 2)
			{
				index = GetLayer(searchLayer)[index].myParentIndex;

				searchLayer++;
			}
			index = backtrackIndex;
			searchLayer = aLayer;
		}
	}
}

bool ASVONVolume::FindLinkInDirection(layerindex aLayer, nodeindex aNodeIndex, uint8 aDir, SVONLink& oLinkToUpdate, FVector& aStartPosForDebug)
{
	int32 maxCoord = GetNodesPerSide(aLayer);
	SVONNode& node = GetLayer(aLayer)[aNodeIndex];
	TArray<SVONNode>& layer = GetLayer(aLayer);

	// Get our world co-ordinate
	uint_fast32_t x = 0, y = 0, z = 0;
	morton3D_64_decode(node.myCode, x, y, z);
	int32 sX = x, sY = y, sZ = z;
	// Add the direction
	sX += SVONStatics::dirs[aDir].X;
	sY += SVONStatics::dirs[aDir].Y;
	sZ += SVONStatics::dirs[aDir].Z;



	// If the coords are out of bounds, the link is invalid.
	if (sX < 0 || sX >= maxCoord || sY < 0 || sY >= maxCoord || sZ < 0 || sZ >= maxCoord)
	{
		oLinkToUpdate.SetInvalid();
		if (myShowNeighbourLinks)
		{
			FVector startPos, endPos;
			GetNodePosition(aLayer, node.myCode, startPos);
			endPos = startPos + (FVector(SVONStatics::dirs[aDir]) * 100.f);
			DrawDebugLine(GetWorld(), aStartPosForDebug, endPos, FColor::Red, true, -1.f, 0, .0f);
		}
		return true;
		//node.myNeighbours[aDir].SetInvalid();
	}
	x = sX; y = sY; z = sZ;
	// Get the morton code for the direction
	mortoncode thisCode = morton3D_64_encode(x, y, z);
	bool isHigher = thisCode > node.myCode;
	int32 idelta = 1;

	// If the code we want is higher, start looking up the array for it
	if (isHigher)
	{
		while (aNodeIndex + idelta < layer.Num())
		{
			// This is the node we're looking for
			if (layer[aNodeIndex + idelta].myCode == thisCode)
			{
				oLinkToUpdate.myLayerIndex = aLayer;
				oLinkToUpdate.myNodeIndex = aNodeIndex + idelta;
				// subnodes???
				if (myShowNeighbourLinks)
				{
					FVector endPos;
					GetNodePosition(aLayer, thisCode, endPos);
					DrawDebugLine(GetWorld(), aStartPosForDebug, endPos, SVONStatics::myLinkColors[aLayer], true, -1.f, 0, .0f);
				}
				return true;
			}
			// If it's higher than the one we want, then it ain't on this layer
			else if (layer[aNodeIndex + idelta].myCode > thisCode)
			{
				return false;
			}

			idelta++;
		}

		return false;
	}
	else // Code is lower, so look down the array
	{
		while (aNodeIndex - idelta >= 0)
		{
			// This is the node we're looking for
			if (layer[aNodeIndex - idelta].myCode == thisCode)
			{
				oLinkToUpdate.myLayerIndex = aLayer;
				oLinkToUpdate.myNodeIndex = aNodeIndex + idelta;
				// subnodes???
				if (myShowNeighbourLinks)
				{
					FVector endPos;
					GetNodePosition(aLayer, thisCode, endPos);
					DrawDebugLine(GetWorld(), aStartPosForDebug, endPos, SVONStatics::myLinkColors[aLayer], true, -1.f, 0, .0f);
				}
				return true;
			}
			// If it's higher than the one we want, then it ain't on this layer
			else if (layer[aNodeIndex - idelta].myCode < thisCode)
			{
				return false;
			}

			idelta++;
		}
		return false;
	}

	UE_LOG(LogTemp, Error, TEXT("Find link reached the end. Layer : %i Node : %i"), aLayer, aNodeIndex);
	return true;

}

void ASVONVolume::RasterizeLeafNode(FVector& aOrigin, nodeindex aLeafIndex)
{
	for (int i = 0; i < 64; i++)
	{

		uint_fast32_t x, y, z;
		morton3D_64_decode(i, x, y, z);
		float leafVoxelSize = GetVoxelSize(0) * 0.25f;
		FVector position = aOrigin + FVector(x * leafVoxelSize, y * leafVoxelSize, z * leafVoxelSize) + FVector(leafVoxelSize * 0.5f);

		if (GetWorld()->OverlapBlockingTestByChannel(position, FQuat::Identity, myCollisionChannel, FCollisionShape::MakeBox(FVector(leafVoxelSize * 0.5f))))
		{
			myLeafNodes[aLeafIndex].SetNodeAt(x, y, z);
			if (myShowVoxels) {
				DrawDebugBox(GetWorld(), position, FVector(leafVoxelSize * 0.5f), FQuat::Identity, FColor::Red, true, -1.f, 0, .0f);
			}

		}
	}
}

TArray<SVONNode>& ASVONVolume::GetLayer(layerindex aLayer)
{
	return myLayers[aLayer];
}


// Check if any nodes within this node's parent is blocked
//   This is unnecessarily slow right now, doing too many iterations, needs changing
bool ASVONVolume::IsAnyMemberBlocked(layerindex aLayer, mortoncode aCode, nodeindex aThisParentIndex, nodeindex& oFirstChildIndex)
{
	int32 parentCode = aCode >> 3;
	bool isBlocked = false;
	for (int32 i = 0; i < GetLayer(aLayer - 1).Num(); i++)
	{
		SVONNode& node = GetLayer(aLayer - 1)[i];
		if (node.myCode >> 3 >> 3 == parentCode)
		{
			if ((node.myCode >> 3) == aCode) {
				node.myParentIndex = aThisParentIndex;
				if (node.myCode % 8 == 0) {
					oFirstChildIndex = i;
				}
			}

			isBlocked = true;
		}
	}
	return isBlocked;
}

// This doesn't work...need to look at it when not tired.
bool ASVONVolume::IsAnyMemberBlocked(layerindex aLayer, mortoncode aCode, nodeindex& oFirstChildIndex)
{
	mortoncode parentCode = aCode >> 3;

	if (aLayer == myBlockedIndices.Num())
	{
		oFirstChildIndex = parentCode << 3;
		return true;
	}
	if (myBlockedIndices[aLayer].Contains(parentCode))
	{
		oFirstChildIndex = parentCode ;
		return true;
	}

	return false;
}

bool ASVONVolume::SetNeighbour(const layerindex aLayer, const nodeindex aArrayIndex, const dir aDirection)
{
	return false;
}


void ASVONVolume::RasterizeLayer(layerindex aLayer)
{
	nodeindex leafIndex = 0;

	// Layer 0 is a special case
	if (aLayer == 0)
	{
		// Run through all our coordinates
		int32 numNodes = GetNodesInLayer(aLayer);
		for (int32 i = 0; i < numNodes; i++)
		{
			int index = (i);

			// If we know this is blocked, from our first pass
			if (myBlockedIndices[0].Contains(i >> 3))
			{
				// Add a node
				index = GetLayer(aLayer).Emplace();
				SVONNode& node = GetLayer(aLayer)[index];

				// Set my code and position
				node.myCode = (i);

				FVector nodePos;
				GetNodePosition(aLayer, node.myCode, nodePos);

				// Debug stuff
				if (myShowMortonCodes) {
					DrawDebugString(GetWorld(), nodePos, FString::FromInt(GetLayer(aLayer)[index].myCode), nullptr, SVONStatics::myLayerColors[aLayer], -1, false);
				}
				if (myShowVoxels) {
					DrawDebugBox(GetWorld(), nodePos, FVector(GetVoxelSize(aLayer) * 0.5f), FQuat::Identity, SVONStatics::myLayerColors[aLayer], true, -1.f, 0, .0f);
				}

				// Rasterize my leaf nodes
				FVector leafOrigin = nodePos - (FVector(GetVoxelSize(aLayer) * 0.5f));
				RasterizeLeafNode(leafOrigin, leafIndex);

				node.myFirstChildIndex = leafIndex;
				leafIndex++;
			}
		}
	}
	// Deal with the other layers
	else if (GetLayer(aLayer - 1).Num() > 1)
	{
		int nodeCounter = 0;
		int32 numNodes = GetNodesInLayer(aLayer);
		for (int32 i = 0; i < numNodes; i++)
		{
			int firstChildIndex = -1;
			// Do we have any blocking children, or siblings?
			// Remember we must have 8 children per parent
			if (IsAnyMemberBlocked(aLayer, i, nodeCounter, firstChildIndex))
			//if(IsAnyMemberBlocked(aLayer, i, firstChildIndex))
			{
				// Add a node
				int32 index = GetLayer(aLayer).Emplace();
				nodeCounter++;
				SVONNode& node = GetLayer(aLayer)[index];
				// Set details
				node.myCode = i;
				if (firstChildIndex > -1)
				{
					node.myFirstChildIndex = firstChildIndex;
				}
				if (myShowMortonCodes || myShowVoxels)
				{
					FVector nodePos;
					GetNodePosition(aLayer, i, nodePos);

					// Debug stuff
					if (myShowVoxels) {
						DrawDebugBox(GetWorld(), nodePos, FVector(GetVoxelSize(aLayer) * 0.5f), FQuat::Identity, SVONStatics::myLayerColors[aLayer], true, -1.f, 0, .0f);
					}
					if (myShowMortonCodes) {
						DrawDebugString(GetWorld(), nodePos, FString::FromInt(GetLayer(aLayer)[index].myCode), nullptr, SVONStatics::myLayerColors[aLayer], -1, false);
					}
				}

			}
		}
	}

}

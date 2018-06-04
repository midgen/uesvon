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
	GetBrushComponent()->Mobility = EComponentMobility::Static;

	BrushColor = FColor(255, 255, 255, 255);

	bColored = true;

	FBox bounds = GetComponentsBoundingBox(true);
	bounds.GetCenterAndExtents(myOrigin, myExtent);
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
	myLeafNodes.AddDefaulted(myBlockedIndices[0].Num() * 8 * 0.25f);

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

	int layerIndex = 0;

	while (myBlockedIndices[layerIndex].Num() > 1)
	{
		// Add a new layer to structure
		myBlockedIndices.Emplace();
		// Add any parent morton codes to the new layer
		for (mortoncode_t& code : myBlockedIndices[layerIndex])
		{
			myBlockedIndices[layerIndex + 1].Add(code >> 3);
		}
		layerIndex++;
	}

	return true;
}

bool ASVONVolume::GetNodePosition(layerindex_t aLayer, mortoncode_t aCode, FVector& oPosition) const
{
	float voxelSize = GetVoxelSize(aLayer);
	uint_fast32_t x, y, z;
	morton3D_64_decode(aCode, x, y, z);
	oPosition = myOrigin - myExtent + FVector(x * voxelSize, y * voxelSize, z * voxelSize) + FVector(voxelSize * 0.5f);
	return true;
}

// Gets the position of a given link. Returns true if the link is open, false if blocked
bool ASVONVolume::GetLinkPosition(SVONLink& aLink, FVector& oPosition) const
{
	const SVONNode& node = GetLayer(aLink.GetLayerIndex())[aLink.GetNodeIndex()];

	GetNodePosition(aLink.GetLayerIndex(), node.myCode, oPosition);
	// If this is layer 0, and there are valid children
	if (aLink.GetLayerIndex() == 0 && node.myFirstChild.IsValid())
	{
		float voxelSize = GetVoxelSize(0);
		uint_fast32_t x,y,z;
		morton3D_64_decode(aLink.GetSubnodeIndex(), x,y,z);
		oPosition += FVector(x * voxelSize * 0.25f, y * voxelSize * 0.25f, z * voxelSize * 0.25f) - FVector(voxelSize * 0.375);
		const SVONLeafNode& leafNode = GetLeafNode(node.myFirstChild.myNodeIndex);
		bool isBlocked = leafNode.GetNode(aLink.GetSubnodeIndex());
		return !isBlocked;
	}
	return true;
}

bool ASVONVolume::GetIndexForCode(layerindex_t aLayer, mortoncode_t aCode, nodeindex_t& oIndex) const
{
	const TArray<SVONNode>& layer = GetLayer(aLayer);

	for (int i = 0; i < layer.Num(); i++)
	{
		if (layer[i].myCode == aCode)
		{
			oIndex = i;
			return true;
		}
	}

	return false;
}

const SVONNode& ASVONVolume::GetNode(const SVONLink& aLink) const
{
	if (aLink.GetLayerIndex() < 14)
	{
		return GetLayer(aLink.GetLayerIndex())[aLink.GetNodeIndex()];
	}
	else
	{
		return GetLayer(myNumLayers - 1)[0];
	}
}

const SVONLeafNode& ASVONVolume::GetLeafNode(nodeindex_t aIndex) const
{
	return myLeafNodes[aIndex];
}

float ASVONVolume::GetVoxelSize(layerindex_t aLayer) const
{
	return (myExtent.X / FMath::Pow(2, myVoxelPower)) * (FMath::Pow(2.0f, aLayer + 1));
}


bool ASVONVolume::IsReadyForNavigation()
{
	return myIsReadyForNavigation;
}


int32 ASVONVolume::GetNodesInLayer(layerindex_t aLayer)
{
	return FMath::Pow(FMath::Pow(2, (myVoxelPower - (aLayer))), 3);
}

int32 ASVONVolume::GetNodesPerSide(layerindex_t aLayer)
{
	return FMath::Pow(2, (myVoxelPower - (aLayer)));
}

void ASVONVolume::BeginPlay()
{
	if (!myIsReadyForNavigation)
	{
		Generate();
		myIsReadyForNavigation = true;
	}
}

void ASVONVolume::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();
}

void ASVONVolume::PostUnregisterAllComponents()
{
	Super::PostUnregisterAllComponents();
}

void ASVONVolume::BuildNeighbourLinks(layerindex_t aLayer)
{

	TArray<SVONNode>& layer = GetLayer(aLayer);
	layerindex_t searchLayer = aLayer;

	// For each node
	for (nodeindex_t i = 0; i < layer.Num(); i++)
	{
		SVONNode& node = layer[i];
		// Get our world co-ordinate
		uint_fast32_t x, y, z;
		morton3D_64_decode(node.myCode, x, y, z);
		nodeindex_t backtrackIndex = -1;
		nodeindex_t index = i;
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
				SVONLink& parent = GetLayer(searchLayer)[index].myParent;
				if (parent.IsValid())
				{
					index = parent.myNodeIndex;
					searchLayer = parent.myLayerIndex;
				}
				else
				{
					searchLayer++;
					GetIndexForCode(searchLayer, node.myCode >> 3, index);
				}

			}
			index = backtrackIndex;
			searchLayer = aLayer;
		}
	}
}

bool ASVONVolume::FindLinkInDirection(layerindex_t aLayer, nodeindex_t aNodeIndex, uint8 aDir, SVONLink& oLinkToUpdate, FVector& aStartPosForDebug)
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
	}
	x = sX; y = sY; z = sZ;
	// Get the morton code for the direction
	mortoncode_t thisCode = morton3D_64_encode(x, y, z);
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

void ASVONVolume::RasterizeLeafNode(FVector& aOrigin, nodeindex_t aLeafIndex)
{
	for (int i = 0; i < 64; i++)
	{

		uint_fast32_t x, y, z;
		morton3D_64_decode(i, x, y, z);
		float leafVoxelSize = GetVoxelSize(0) * 0.25f;
		FVector position = aOrigin + FVector(x * leafVoxelSize, y * leafVoxelSize, z * leafVoxelSize) + FVector(leafVoxelSize * 0.5f);

		if (aLeafIndex >= myLeafNodes.Num() - 1)
			myLeafNodes.AddDefaulted(1);

		if (GetWorld()->OverlapBlockingTestByChannel(position, FQuat::Identity, myCollisionChannel, FCollisionShape::MakeBox(FVector(leafVoxelSize * 0.5f))))
		{
			myLeafNodes[aLeafIndex].SetNode(i);
			if (myShowVoxels) {
				DrawDebugBox(GetWorld(), position, FVector(leafVoxelSize * 0.5f), FQuat::Identity, FColor::Red, true, -1.f, 0, .0f);
			}

		}
	}
}

TArray<SVONNode>& ASVONVolume::GetLayer(layerindex_t aLayer)
{
	return myLayers[aLayer];
}

const TArray<SVONNode>& ASVONVolume::GetLayer(layerindex_t aLayer) const
{
	return myLayers[aLayer];
}

// Check for blocking...using this cached set for each layer for now for fast lookups
bool ASVONVolume::IsAnyMemberBlocked(layerindex_t aLayer, mortoncode_t aCode)
{
	mortoncode_t parentCode = aCode >> 3;

	if (aLayer == myBlockedIndices.Num())
	{
		return true;
	}
	// The parent of this code is blocked
	if (myBlockedIndices[aLayer].Contains(parentCode))
	{
		return true;
	}

	return false;
}

bool ASVONVolume::SetNeighbour(const layerindex_t aLayer, const nodeindex_t aArrayIndex, const dir aDirection)
{
	return false;
}


void ASVONVolume::RasterizeLayer(layerindex_t aLayer)
{
	nodeindex_t leafIndex = 0;
	// Layer 0 Leaf nodes are special
	if (aLayer == 0)
	{
		// Run through all our coordinates
		int32 numNodes = GetNodesInLayer(aLayer);
		for (int32 i = 0; i < numNodes; i++)
		{
			int index = (i);

			// If we know this node needs to be added, from the low res first pass
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
					DrawDebugString(GetWorld(), nodePos, FString::FromInt(node.myCode), nullptr, SVONStatics::myLayerColors[aLayer], -1, false);
				}
				if (myShowVoxels) {
					DrawDebugBox(GetWorld(), nodePos, FVector(GetVoxelSize(aLayer) * 0.5f), FQuat::Identity, SVONStatics::myLayerColors[aLayer], true, -1.f, 0, .0f);
				}
				
				// Now check if we have any blocking, and search leaf nodes
				FVector position;
				GetNodePosition(0, i, position);

				if (GetWorld()->OverlapBlockingTestByChannel(position, FQuat::Identity, myCollisionChannel, FCollisionShape::MakeBox(FVector(GetVoxelSize(0) * 0.5f))))
				{
					// Rasterize my leaf nodes
					FVector leafOrigin = nodePos - (FVector(GetVoxelSize(aLayer) * 0.5f));
					RasterizeLeafNode(leafOrigin, leafIndex);
					node.myFirstChild.SetLayerIndex(0);
					node.myFirstChild.SetNodeIndex(leafIndex);
					node.myFirstChild.SetSubnodeIndex(0);
					leafIndex++;
				}
				else
				{
					node.myFirstChild.SetInvalid();
				}

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
			// Do we have any blocking children, or siblings?
			// Remember we must have 8 children per parent
			if (IsAnyMemberBlocked(aLayer, i))
			{
				// Add a node
				int32 index = GetLayer(aLayer).Emplace();
				nodeCounter++;
				SVONNode& node = GetLayer(aLayer)[index];
				// Set details
				node.myCode = i;
				nodeindex_t childIndex = 0;
				if (GetIndexForCode(aLayer - 1, node.myCode << 3, childIndex))
				{
					// Set parent->child links
					node.myFirstChild.SetLayerIndex(aLayer - 1);
					node.myFirstChild.SetNodeIndex(childIndex);
					// Set child->parent links, this can probably be done smarter, as we're duplicating work here
					for (int iter = 0; iter < 8; iter++)
					{
						GetLayer(node.myFirstChild.GetLayerIndex())[node.myFirstChild.GetNodeIndex() + iter].myParent.SetLayerIndex(aLayer);
						GetLayer(node.myFirstChild.GetLayerIndex())[node.myFirstChild.GetNodeIndex() + iter].myParent.SetNodeIndex(index);
					}
					
					if (myShowParentChildLinks) // Debug all the things
					{
						FVector startPos, endPos;
						GetNodePosition(aLayer, node.myCode, startPos);
						GetNodePosition(aLayer - 1, node.myCode << 3, endPos);
						DrawDebugDirectionalArrow(GetWorld(), startPos, endPos, 0.f, SVONStatics::myLinkColors[aLayer], true);
					}
				}
				else
				{
					node.myFirstChild.SetInvalid();
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
						DrawDebugString(GetWorld(), nodePos, FString::FromInt(node.myCode), nullptr, SVONStatics::myLayerColors[aLayer], -1, false);
					}
				}

			}
		}
	}

}

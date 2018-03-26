// Fill out your copyright notice in the Description page of Project Settings.

#include "SVONManager.h"
#include "libmorton/morton.h"
#include "DrawDebugHelpers.h"

// Sets default values
ASVONManager::ASVONManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void ASVONManager::AllocateLeafNodes()
{
	myLeafNodes.Empty();
	myLeafNodes.Reserve(myBlockedIndices.Num() * 8);
	myLeafNodes.AddDefaulted(myBlockedIndices.Num() * 8);
}

void ASVONManager::Generate()
{
	myBlockedIndices.Empty();
	
	FirstPassRasterize();

	AllocateLeafNodes();

	for (int i = 0; i < NUM_LAYERS; i++)
	{
		RasterizeLayer(i);
	}

	for (int i = NUM_LAYERS; i >= 0; i--)
	{
		BuildNeighbourLinks(i);
	}
}

TArray<SVONNode>& ASVONManager::GetLayer(layerindex aLayer)
{
	return myLayers[aLayer];
}

int32 ASVONManager::GetLayerSize(layerindex aLayer)
{
	return FMath::Pow(2, myVoxelPower) / (aLayer > 0 ? 2 * aLayer : 1);
}

int32 ASVONManager::GetNodesInLayer(layerindex aLayer)
{
	return FMath::Pow(GetLayerSize(aLayer), 3); 
}

void ASVONManager::FirstPassRasterize()
{
	int32 numNodes = GetNodesInLayer(1);
	for (int32 i = 0; i < numNodes; i++)
	{
		FVector position;
		GetNodePosition(1, i, position);

		if (GetWorld()->OverlapBlockingTestByChannel(position, FQuat::Identity, myCollisionChannel, FCollisionShape::MakeBox(FVector(myVoxelSize[1] * 0.5f))))
		{
			myBlockedIndices.Add(i);
		}
	}

	myBlockedIndices.Sort([](const uint_fast64_t& A, const uint_fast64_t& B) {
		return B > A; 
	});
}

void ASVONManager::RasterizeLayer(layerindex aLayer)
{
	nodeindex leafIndex = 0;

	// Layer 0 is a special case
	if (aLayer == 0)
	{
		// Run through all our coordinates
		int32 numNodes = GetNodesInLayer(aLayer);
		for(int32 i = 0; i < numNodes; i++)
		{
			int index = (i);

			// If we know this is blocked, from our first pass
			if (myBlockedIndices.Contains(i >> 3))
			{
				// Add a node
				index = GetLayer(aLayer).Emplace();
				SVONNode& node = GetLayer(aLayer)[index];

				// Set my code and position
				node.myCode = (i);
				GetNodePosition(aLayer, node.myCode, node.myPosition);

				// Debug stuff
				if (myShowMortonCodes) { 
					DrawDebugString(GetWorld(), node.myPosition, FString::FromInt(GetLayer(aLayer)[index].myCode), nullptr, myLayerColors[aLayer], -1, false); 
				}
				if (myShowVoxels) {
					DrawDebugBox(GetWorld(), node.myPosition, FVector(myVoxelSize[aLayer] * 0.5f), FQuat::Identity, myLayerColors[aLayer], true, -1.f, 0, aLayer + 1 * 6.0f);
				}
				
				// Rasterize my leaf nodes
				FVector leafOrigin = node.myPosition - (FVector(myVoxelSize[aLayer]) * 0.5f);
				RasterizeLeafNode(leafOrigin, leafIndex);

				node.myFirstChildIndex = leafIndex;
				leafIndex++;
			}
		}
	}
	else 
	{	
		int nodeCounter = 0;
		int32 numNodes = GetNodesInLayer(aLayer);
		for (int32 i = 0; i < numNodes; i++)
		{
			int firstChildIndex = -1;
			if (IsAnyMemberBlocked(aLayer, i, nodeCounter, firstChildIndex))
			{
				int32 index = GetLayer(aLayer).Emplace();
				nodeCounter++;
				SVONNode& node = GetLayer(aLayer)[index];

				node.myCode = i;
				if (firstChildIndex > -1)
				{
					node.myFirstChildIndex = firstChildIndex;
				}
				GetNodePosition(aLayer, i, node.myPosition);
				//node.myFirstChild.SetLayerIndex(aLayer - 1);
				if (myShowLinks && firstChildIndex > -1) {
					DrawDebugDirectionalArrow(GetWorld(), node.myPosition, GetLayer(aLayer - 1)[node.myFirstChildIndex].myPosition, 20.0f, myLayerColors[aLayer], true, -1.f, 0, 20.0f);
				}
				if (myShowVoxels) {
					DrawDebugBox(GetWorld(), node.myPosition, FVector(myVoxelSize[aLayer] * 0.5f), FQuat::Identity, myLayerColors[aLayer], true, -1.f, 0, aLayer + 1 * 6.0f);
				}
				if (myShowMortonCodes) {
					DrawDebugString(GetWorld(), node.myPosition, FString::FromInt(GetLayer(aLayer)[index].myCode), nullptr, myLayerColors[aLayer], -1, false);
				}
				
			}
		}
	}

}

// Check if any nodes within this node's parent is blocked
bool ASVONManager::IsAnyMemberBlocked(layerindex aLayer, mortoncode aCode, nodeindex aThisParentIndex, nodeindex& oFirstChildIndex)
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
			
			isBlocked =  true;
		}
	}
	return isBlocked;
}

void ASVONManager::BuildNeighbourLinks(layerindex aLayer)
{

}

void ASVONManager::RasterizeLeafNode(FVector& aOrigin, nodeindex aLeafIndex)
{
	for (int i = 0; i < 64; i++)
	{

		uint_fast32_t x, y, z;
		morton3D_64_decode(i, x, y, z);
		float leafVoxelSize = myVoxelSize[0] * 0.25f;
		FVector position = aOrigin + FVector(x * leafVoxelSize, y * leafVoxelSize, z * leafVoxelSize) + FVector(leafVoxelSize * 0.5f);

		if (i == 0)
		{
			//DrawDebugDirectionalArrow(GetWorld(), position, aOrigin, 2.0f, FColor::Red, true, -1.f, 0, 2.0f);
		}
		if (GetWorld()->OverlapBlockingTestByChannel(position, FQuat::Identity, myCollisionChannel, FCollisionShape::MakeBox(FVector(leafVoxelSize * 0.5f))))
		{
			myLeafNodes[aLeafIndex].SetNodeAt(x,y,z);
			if (myShowVoxels) {
				DrawDebugBox(GetWorld(), position, FVector(leafVoxelSize * 0.5f), FQuat::Identity, FColor::Red, true, -1.f, 0, 6.0f);
			}
			
		}
	}
}



bool ASVONManager::SetNeighbour(const layerindex aLayer, const nodeindex aArrayIndex, const dir aDirection)
{
	return false;
	//GetLayer(aLayer)[aArrayIndex]
}

SVONNode& ASVONManager::GetNodeFromLink(SVONLink& aLink)
{
	return GetLayer(aLink.GetLayerIndex())[aLink.GetNodeIndex()];
}

// Called when the game starts or when spawned
void ASVONManager::BeginPlay()
{
	Super::BeginPlay();

	FBox bounds = myBoundsVolume->GetComponentsBoundingBox(true);
	bounds.GetCenterAndExtents(myOrigin, myExtent);

	myVoxelSize[0] = (myExtent.X / FMath::Pow(2, myVoxelPower)) *2.0f;
	myVoxelSize[1] = myVoxelSize[0] * 2.0f;
	myVoxelSize[2] = myVoxelSize[1] * 2.0f;
	myVoxelSize[3] = myVoxelSize[2] * 2.0f;


	//myLayerSize[0] = FMath::Pow(2, myVoxelPower);
	//myLayerSize[1] = myLayerSize[0] / 2;
	//myLayerSize[2] = myLayerSize[1] / 2;
	//myLayerSize[3] = myLayerSize[2] / 2;


	Generate();

	
}

bool ASVONManager::GetNodePosition(layerindex aLayer, mortoncode aCode, FVector& oPosition)
{
	if (aLayer > NUM_LAYERS)
	{
		return false;
	}
	uint_fast32_t x, y, z;
	morton3D_64_decode(aCode, x, y, z);
	oPosition = myOrigin - myExtent + FVector(x * myVoxelSize[aLayer], y * myVoxelSize[aLayer], z * myVoxelSize[aLayer]) + FVector(myVoxelSize[aLayer] * 0.5f);
	return true;
}

SVONNode& ASVONManager::GetNodeAt(layerindex aLayer, posint aX, posint aY, posint aZ)
{
	uint_fast64_t index = 0;
	morton3D_64_decode(index, aX, aY, aZ);
	return GetLayer(aLayer)[index];
}

// Called every frame
void ASVONManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


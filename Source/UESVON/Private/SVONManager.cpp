// Fill out your copyright notice in the Description page of Project Settings.

#include "SVONManager.h"
#include "libmorton/morton.h"
#include "DrawDebugHelpers.h"

// Sets default values
ASVONManager::ASVONManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ASVONManager::AllocateLayerNodes()
{
	for (int i = 0; i < NUM_LAYERS; i++)
	{
	/*	GetLayer(i).Reserve(FMath::Pow(myLayerSize[i], 3));
		for (int j = 0; j < FMath::Pow(myLayerSize[i], 3); j++)
		{
			GetLayer(i).Emplace(i);
		}*/
		
	}
	
	myBlockedIndices.Empty();
}

void ASVONManager::AllocateLeafNodes()
{
	myLeafNodes.Empty();
	myLeafNodes.Reserve(myBlockedIndices.Num() * 8);
	myLeafNodes.AddDefaulted(myBlockedIndices.Num() * 8);
}

void ASVONManager::Generate()
{
	AllocateLayerNodes();

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

TArray<SVONNode>& ASVONManager::GetLayer(uint8 aLayer)
{
	return myLayers[aLayer];
}

void ASVONManager::FirstPassRasterize()
{
	for (uint_fast64_t i = 0; i < FMath::Pow(myLayerSize[1], 3); i++)
	{
		FVector position;	//= myOrigin - myExtent + FVector(x * myVoxelSize[1], y * myVoxelSize[1], z * myVoxelSize[1]) + FVector(myVoxelSize[1] * 0.5f);
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

void ASVONManager::RasterizeLayer(uint8 aLayer)
{
	uint_fast64_t leafIndex = 0;

	if (aLayer == 0)
	{
		int parentIndex = 0;
		bool isThis8Blocked = false;
		for(uint32 i = 0; i < FMath::Pow(myLayerSize[aLayer], 3); i++)
		{
			int index = (i);
			
				
			//UWorld* CurrentWorld = GetWorld();
			//if (CurrentWorld->OverlapBlockingTestByChannel(GetLayer(aLayer)[i].myPosition, FQuat::Identity, myCollisionChannel, FCollisionShape::MakeBox(FVector(myVoxelSize[aLayer] * 0.5f))))


			// If we know this is blocked, from our first pass
			if (myBlockedIndices.Contains(i >> 3))
			{					
				isThis8Blocked = true;
				// If we're the first child of a blocked parent, add the parent and set links
		/*		if (i % 8 == 0)
				{
					parentIndex = GetLayer(aLayer + 1).Emplace();

					GetLayer(aLayer + 1)[parentIndex].myCode = i >> 3;
					GetNodePosition(aLayer + 1, i >> 3, GetLayer(aLayer + 1)[parentIndex].myPosition);
					GetLayer(aLayer + 1)[parentIndex].myFirstChild.SetLayerIndex(aLayer);
					GetLayer(aLayer + 1)[parentIndex].myFirstChild.SetNodeIndex(i);
				}*/
				index = GetLayer(aLayer).Emplace();

				// Set my code and position
				GetLayer(aLayer)[index].myCode = (i);

				GetNodePosition(aLayer, GetLayer(aLayer)[index].myCode, GetLayer(aLayer)[index].myPosition);
				//DrawDebugString(GetWorld(), GetLayer(aLayer)[index].myPosition, FString::FromInt(GetLayer(aLayer)[index].myCode), nullptr, FColor::Red, -1, false);
				DrawDebugBox(GetWorld(), GetLayer(aLayer)[index].myPosition, FVector(myVoxelSize[aLayer] * 0.5f), FQuat::Identity, myLayerColors[aLayer], true, -1.f, 0, aLayer + 1 * 6.0f);

				// Rasterize my leaf nodes
				FVector leafOrigin = GetLayer(aLayer)[index].myPosition - (FVector(myVoxelSize[aLayer]) * 0.5f);
				RasterizeLeafNode(leafOrigin, leafIndex);
				leafIndex++;

				// parent stuff
				//GetLayer(aLayer)[index].myParent.SetLayerIndex(aLayer + 1);
				//GetLayer(aLayer)[index].myParent.SetNodeIndex(parentIndex);

				//DrawDebugDirectionalArrow(GetWorld(), GetLayer(aLayer + 1)[parentIndex].myPosition, GetLayer(aLayer)[index].myPosition, 50.0f, FColor::Cyan, true, -1.f, 0, 20.0f);

			}
			// Otherwise, we're not blocked at all, so add an unlinked parent
	/*		else if (i % 8 == 7 && !isThis8Blocked)
			{
				parentIndex = GetLayer(aLayer + 1).Emplace();

				GetLayer(aLayer + 1)[parentIndex].myCode = i >> 3;
				GetNodePosition(aLayer + 1, i >> 3, GetLayer(aLayer + 1)[parentIndex].myPosition);
				isThis8Blocked = false;
			}*/

		}
	}
	else 
	{
		
		int maxParentCode = -1;
		int parentIndex = 0;
		
		for (int32 i = 0; i < FMath::Pow(myLayerSize[aLayer],3); i++)
		{
// 			for (int32 j = 0; j < GetLayer(aLayer-1).Num(); j++)
// 			{
			if (IsAnyMemberBlocked(aLayer, i))// && (i >> 3) > maxParentCode )
				{
					int32 index = GetLayer(aLayer).Emplace();
					//maxParentCode = i;

					GetLayer(aLayer)[index].myCode = i;
					GetNodePosition(aLayer, i, GetLayer(aLayer)[index].myPosition);
					GetLayer(aLayer)[index].myFirstChild.SetLayerIndex(aLayer - 1);
					GetLayer(aLayer)[index].myFirstChild.SetNodeIndex(i);
					DrawDebugBox(GetWorld(), GetLayer(aLayer)[index].myPosition, FVector(myVoxelSize[aLayer] * 0.5f), FQuat::Identity, myLayerColors[aLayer], true, -1.f, 0, aLayer + 1 * 6.0f);
				}

			//}
		}
	}

}

// Check if any nodes within this node's parent is blocked
bool ASVONManager::IsAnyMemberBlocked(uint8 aLayer, uint_fast64_t aCode)
{
	int32 parentCode = aCode >> 3;
	for (int32 i = 0; i < GetLayer(aLayer - 1).Num(); i++)
	{
		if (GetLayer(aLayer - 1)[i].myCode >> 3 >> 3 == parentCode)
		{
			return true;
		}
	}
	return false;
}

void ASVONManager::BuildNeighbourLinks(uint8 aLayer)
{

}

void ASVONManager::RasterizeLeafNode(FVector& aOrigin, uint_fast64_t aLeafIndex)
{
	for (int i = 0; i < 64; i++)
	{

		uint_fast32_t x, y, z;
		morton3D_64_decode(i, x, y, z);
		float leafVoxelSize = myVoxelSize[0] * 0.25f;
		FVector position = aOrigin + FVector(x * leafVoxelSize, y * leafVoxelSize, z * leafVoxelSize) + FVector(leafVoxelSize * 0.5f);

		if (i == 0)
		{
			DrawDebugDirectionalArrow(GetWorld(), position, aOrigin, 2.0f, FColor::Red, true, -1.f, 0, 2.0f);
		}
		if (GetWorld()->OverlapBlockingTestByChannel(position, FQuat::Identity, myCollisionChannel, FCollisionShape::MakeBox(FVector(leafVoxelSize * 0.5f))))
		{
			myLeafNodes[aLeafIndex].SetNodeAt(x,y,z);
			DrawDebugBox(GetWorld(), position, FVector(leafVoxelSize * 0.5f), FQuat::Identity, FColor::Red, true, -1.f, 0, 6.0f);
		}
		else
		{
			//DrawDebugBox(GetWorld(), position, FVector(leafVoxelSize * 0.5f), FQuat::Identity, FColor::White, true, -1.f, 0, 6.0f);
		}
		//DrawDebugString(GetWorld(), position, FString::FromInt(i), nullptr, FColor::Red, -1, false);
	}
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


	myLayerSize[0] = FMath::Pow(2, myVoxelPower);
	myLayerSize[1] = myLayerSize[0] / 2;
	myLayerSize[2] = myLayerSize[1] / 2;
	myLayerSize[3] = myLayerSize[2] / 2;


	Generate();

	
}

bool ASVONManager::GetNodePosition(uint8 aLayer, uint_fast64_t aCode, FVector& oPosition)
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

SVONNode& ASVONManager::GetNodeAt(uint8 aLayer, uint_fast32_t aX, uint_fast32_t aY, uint_fast32_t aZ)
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


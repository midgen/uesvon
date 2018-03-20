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
		GetLayer(i).AddDefaulted(FMath::Pow(myLayerSize[i], 3));
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

}

TArray<SVONNode>& ASVONManager::GetLayer(uint8 aLayer)
{

	return myLayers[aLayer];
	//switch (aLayer)
	//{
	//case 0: return myLayer0;
	//	break;
	//case 1: return myLayer1;
	//	break;
	//case 2: return myLayer2;
	//	break;
	//}

	//return myLayer0;
}

void ASVONManager::FirstPassRasterize()
{
	for (uint_fast64_t i = 0; i < GetLayer(1).Num(); i++)
	{
		uint_fast32_t x, y, z;
		morton3D_64_decode(i, x, y, z);
		FVector position;	//= myOrigin - myExtent + FVector(x * myVoxelSize[1], y * myVoxelSize[1], z * myVoxelSize[1]) + FVector(myVoxelSize[1] * 0.5f);
		GetNodePosition(1, i, position);

		// Rasterize the volume (placeholder)

		if (GetWorld()->OverlapBlockingTestByChannel(position, FQuat::Identity, myCollisionChannel, FCollisionShape::MakeBox(FVector(myVoxelSize[1] * 0.5f))))
		{
			myBlockedIndices.Add(i);
			//DrawDebugBox(GetWorld(), position, FVector(myVoxelSize[1] * 0.5f), FQuat::Identity, FColor::White, true, -1.f, 0, 6.0f );
		}
		else
		{
			//DrawDebugBox(GetWorld(), position, FVector(myVoxelSize[1] * 0.5f), FQuat::Identity, FColor::White, true, -1.f, 0, 6.0f);
			
		}
		//DrawDebugString(CurrentWorld, position, FString::FromInt(i), nullptr, FColor::White, -1, false);
	}


}

void ASVONManager::RasterizeLayer(uint8 aLayer)
{
	uint_fast64_t leafIndex = 0;

	for (uint_fast64_t i = 0; i < GetLayer(aLayer).Num(); i++)
	{
		//uint_fast32_t x, y, z;
		//morton3D_64_decode(i, x, y, z);
		//myLayers[aLayer][i].myPosition = myOrigin - myExtent + FVector(x * myVoxelSize[aLayer], y * myVoxelSize[aLayer], z * myVoxelSize[aLayer]) + FVector(myVoxelSize[aLayer] * 0.5f);

		GetNodePosition(aLayer, i, GetLayer(aLayer)[i].myPosition);

		// Rasterize the volume (placeholder)
		UWorld* CurrentWorld = GetWorld();
		if (CurrentWorld->OverlapBlockingTestByChannel(GetLayer(aLayer)[i].myPosition, FQuat::Identity, myCollisionChannel, FCollisionShape::MakeBox(FVector(myVoxelSize[aLayer] * 0.5f))))
		{
			DrawDebugBox(CurrentWorld, GetLayer(aLayer)[i].myPosition, FVector(myVoxelSize[aLayer] * 0.5f), FQuat::Identity, myLayerColors[aLayer], true, -1.f, 0, aLayer * 6.0f);
			if (aLayer == 0)
			{
				FVector leafOrigin = GetLayer(aLayer)[i].myPosition - (FVector(myVoxelSize[aLayer]) * 0.5f);
				RasterizeLeafNode(leafOrigin, leafIndex);
				leafIndex++;
			}
			if (aLayer < NUM_LAYERS - 1)
			{
				GetLayer(aLayer)[i].myParent.myLayerIndex = aLayer + 1;
				GetLayer(aLayer)[i].myParent.myNodeIndex = i >> 3;
				FVector parentPosition;
				GetNodePosition(aLayer + 1, i >> 3, parentPosition);
				DrawDebugDirectionalArrow(GetWorld(), GetLayer(aLayer)[i].myPosition, parentPosition, 20.0f, myLinkColors[aLayer], true, -1.f, 0, (aLayer + 1) * 10.0f);
			}
		}


		//DrawDebugString(CurrentWorld, GetLayer(aLayer)[i].myPosition, FString::FromInt(GetLayer(aLayer)[i].myParent.myNodeIndex), nullptr, myLayerColors[aLayer], -1, false);
	}
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

bool ASVONManager::GetNodePosition(uint8 aLayer, uint_fast64_t aIndex, FVector& oPosition)
{
	if (aLayer > NUM_LAYERS)
	{
		return false;
	}
	uint_fast32_t x, y, z;
	morton3D_64_decode(aIndex, x, y, z);
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


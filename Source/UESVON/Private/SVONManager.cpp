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

void ASVONManager::Allocate()
{
	for (int i = 0; i < NUM_LAYERS; i++)
	{
		myLayers[i].Empty();
		myLayers[i].AddDefaulted(FMath::Pow(myLayerSize[i], 3));
	}
	
	myBlockedIndices.Empty();
}

void ASVONManager::Generate()
{
	FirstPassRasterize();

	RasterizeLayer(0);
	

}

void ASVONManager::FirstPassRasterize()
{
	for (uint_fast64_t i = 0; i < myLayers[1].Num(); i++)
	{
		uint_fast32_t x, y, z;
		morton3D_64_decode(i, x, y, z);
		FVector position = myOrigin - myExtent + FVector(x * myVoxelSize[1], y * myVoxelSize[1], z * myVoxelSize[1]) +FVector(myVoxelSize[1] * 0.5f);
		DrawDebugString(GetWorld(), myOrigin, "Origin", nullptr, FColor::White, -1, false);

		// Rasterize the volume (placeholder)
		UWorld* CurrentWorld = GetWorld();
		if (CurrentWorld->OverlapBlockingTestByChannel(position, FQuat::Identity, myCollisionChannel, FCollisionShape::MakeBox(FVector(myVoxelSize[1] * 0.5f))))
		{
			myBlockedIndices.Add(i);
			DrawDebugBox(CurrentWorld, position, FVector(myVoxelSize[1] * 0.5f), FQuat::Identity, FColor::Yellow, true, -1.f, 0, 6.0f );
		}
		else
		{
			DrawDebugBox(CurrentWorld, position, FVector(myVoxelSize[1] * 0.5f), FQuat::Identity, FColor::White, true, -1.f, 0, 6.0f);
			
		}
		DrawDebugString(CurrentWorld, position, FString::FromInt(i), nullptr, FColor::White, -1, false);
	}

	myLeafNodes.Empty();
	myLeafNodes.Reserve(myBlockedIndices.Num() * 8);
	myLeafNodes.AddDefaulted(myBlockedIndices.Num() * 8);
}

void ASVONManager::RasterizeLayer(uint8 aLayer)
{
	uint_fast64_t leafIndex = 0;

	for (uint_fast64_t i = 0; i < myLayers[aLayer].Num(); i++)
	{
		uint_fast32_t x, y, z;
		morton3D_64_decode(i, x, y, z);
		myLayers[aLayer][i].myPosition = myOrigin - myExtent + FVector(x * myVoxelSize[aLayer], y * myVoxelSize[aLayer], z * myVoxelSize[aLayer]) + FVector(myVoxelSize[aLayer] * 0.5f);

		// Rasterize the volume (placeholder)
		UWorld* CurrentWorld = GetWorld();
		if (CurrentWorld->OverlapBlockingTestByChannel(myLayers[aLayer][i].myPosition, FQuat::Identity, myCollisionChannel, FCollisionShape::MakeBox(FVector(myVoxelSize[aLayer] * 0.5f))))
		{
			DrawDebugBox(CurrentWorld, myLayers[aLayer][i].myPosition, FVector(myVoxelSize[aLayer] * 0.5f), FQuat::Identity, FColor::Orange, true, -1.f, 0, 6.0f);
			if (aLayer == 0)
			{
				FVector leafOrigin = myLayers[aLayer][i].myPosition - (FVector(myVoxelSize[aLayer]) * 0.5f);
				RasterizeLeafNode(leafOrigin, leafIndex);
				leafIndex++;
			}
		}
		DrawDebugString(CurrentWorld, myLayers[aLayer][i].myPosition, FString::FromInt(i), nullptr, FColor::Orange, -1, false);
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
		if (GetWorld()->OverlapBlockingTestByChannel(position, FQuat::Identity, myCollisionChannel, FCollisionShape::MakeBox(FVector(leafVoxelSize * 0.5f))))
		{
			myLeafNodes[aLeafIndex].SetNodeAt(x,y,z);
			DrawDebugBox(GetWorld(), position, FVector(leafVoxelSize * 0.5f), FQuat::Identity, FColor::Red, true, -1.f, 0, 6.0f);
		}
		else
		{
			//DrawDebugBox(GetWorld(), position, FVector(leafVoxelSize * 0.5f), FQuat::Identity, FColor::White, true, -1.f, 0, 6.0f);
		}
		DrawDebugString(GetWorld(), position, FString::FromInt(i), nullptr, FColor::Red, -1, false);
	}
}

// Called when the game starts or when spawned
void ASVONManager::BeginPlay()
{
	Super::BeginPlay();

	FBox bounds = myBoundsVolume->GetComponentsBoundingBox(true);
	bounds.GetCenterAndExtents(myOrigin, myExtent);

	//int32 dimX, dimY, dimZ;

	myVoxelSize[0] = (myExtent.X / FMath::Pow(2, myVoxelPower)) *2.0f;
	myVoxelSize[1] = myVoxelSize[0] * 2.0f;

	myLayerSize[0] = FMath::Pow(2, myVoxelPower);
	myLayerSize[1] = myLayerSize[0] / 2;


	//GEngine->AddOnScreenDebugMessage(0, 5.f, FColor::Red, TEXT("Voxels - " + FString::FromInt(dimX) + ":" + FString::FromInt(dimY) + ":" + FString::FromInt(dimZ) + ":"));


	Allocate();

	Generate();

	
}

SVONNode& ASVONManager::GetNodeAt(uint8 aLayer, uint_fast32_t aX, uint_fast32_t aY, uint_fast32_t aZ)
{
	uint_fast64_t index = 0;
	morton3D_64_decode(index, aX, aY, aZ);
	return myLayers[aLayer][index];
}

// Called every frame
void ASVONManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


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

void ASVONManager::Allocate(const int32 x, const int32 y, const int32 z)
{
	myNodes.Empty();

	int32 numNodes = x * y * z;

	myNodes.Reserve(numNodes);
	myNodes.AddDefaulted(numNodes);
}

void ASVONManager::Generate()
{
	for (uint_fast64_t i = 0; i < myNodes.Num(); i++)
	{
		uint_fast32_t x, y, z;
		morton3D_64_decode(i, x, y, z);
		myNodes[i].myPosition.X = myOrigin.X - myExtent.X + (x * myVoxelSize) + (myVoxelSize * 0.5f);
		myNodes[i].myPosition.Y = myOrigin.Y - myExtent.Y + (y * myVoxelSize) + (myVoxelSize * 0.5f);
		myNodes[i].myPosition.Z = myOrigin.Z - myExtent.Z + (z * myVoxelSize) + (myVoxelSize * 0.5f);
		GEngine->AddOnScreenDebugMessage(0, 5.f, FColor::Red, TEXT("Morton: - " + FString::FromInt(i) + ":" + FString::FromInt(x) + ":" + FString::FromInt(y) + ":" + FString::FromInt(z)));

		// Rasterize the volume (placeholder)
		UWorld* CurrentWorld = GetWorld();
		if (CurrentWorld->OverlapBlockingTestByChannel(myNodes[i].myPosition, FQuat::Identity, myCollisionChannel, FCollisionShape::MakeBox(FVector(myVoxelSize * 0.5f))))
		{
			myNodes[i].myIsBlocked = true;
			DrawDebugBox(CurrentWorld, myNodes[i].myPosition, FVector(myVoxelSize * 0.5f), FQuat::Identity, myNodes[i].myIsBlocked ? FColor::Red : FColor::White, true, -1.f, 0, myNodes[i].myIsBlocked ? 6.0f : 4.0f);
		}

		// Debug draw (needs to be faster)
		
		//DrawDebugSphere(CurrentWorld, myNodes[i].myPosition, myVoxelSize * 0.5f, 8, myNodes[i].myIsBlocked ? FColor::Red : FColor::White, true, -1.f, 0,0.f);
	}
}

// Called when the game starts or when spawned
void ASVONManager::BeginPlay()
{
	Super::BeginPlay();

	FBox bounds = myBoundsVolume->GetComponentsBoundingBox(true);
	bounds.GetCenterAndExtents(myOrigin, myExtent);

	int32 dimX, dimY, dimZ;

	myVoxelSize = (myExtent.X / FMath::Pow(2, myVoxelPower)) * 2.0f;

	dimX = FMath::Pow(2, myVoxelPower);
	dimY = dimX;
	dimZ = dimX;

	GEngine->AddOnScreenDebugMessage(0, 5.f, FColor::Red, TEXT("Voxels - " + FString::FromInt(dimX) + ":" + FString::FromInt(dimY) + ":" + FString::FromInt(dimZ) + ":"));

	Allocate(dimX, dimY, dimZ);

	Generate();

	
}

SVONNode& ASVONManager::GetNodeAt(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z)
{
	uint_fast64_t index = 0;
	morton3D_64_decode(index, x, y, z);
	return myNodes[index];
}

// Called every frame
void ASVONManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


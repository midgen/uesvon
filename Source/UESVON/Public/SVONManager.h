// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SVONNode.h"
#include "SVONLeafNode.h"
#include "SVONManager.generated.h"

UCLASS()
class UESVON_API ASVONManager : public AActor
{
	GENERATED_BODY()

public:
	static const uint8 NUM_LAYERS = 2;

private:
	TArray<SVONNode> myLayers[NUM_LAYERS];
	TArray<SVONLeafNode> myLeafNodes;
	TSet<uint_fast64_t> myBlockedIndices;

	void Allocate();

	void FirstPassRasterize();
	void RasterizeLayer(uint8 aLayer);
	void RasterizeLeafNode(FVector& aOrigin, uint_fast64_t aLeafIndex);

	void Generate();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	SVONNode& GetNodeAt(uint8 aLayer, uint_fast32_t aX, uint_fast32_t aY, uint_fast32_t aZ);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Sets default values for this actor's properties
	ASVONManager();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ASVONBoundsVolume* myBoundsVolume;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
		uint8 myVoxelPower;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
	TEnumAsByte<ECollisionChannel> myCollisionChannel;


	// Voxel dimensions. 0 is leaf node.
	float myVoxelSize[NUM_LAYERS];
	uint32 myLayerSize[NUM_LAYERS];

	FVector myOrigin;
	FVector myExtent;


};

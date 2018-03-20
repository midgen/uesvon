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

	static const uint8 NUM_LAYERS = 4;

private:
	//TArray<SVONNode> myLayer0;
	//TArray<SVONNode> myLayer1;
	//TArray<SVONNode> myLayer2;
	//TArray<SVONNode> myLayer3;

	TArray<SVONNode> myLayers[NUM_LAYERS];

	TArray<SVONLeafNode> myLeafNodes;
	TSet<uint_fast64_t> myBlockedIndices;

	FColor myLayerColors[NUM_LAYERS] = { FColor::Orange, FColor::Yellow, FColor::White, FColor::Magenta  };
	FColor myLinkColors[NUM_LAYERS] = { FColor::Cyan, FColor::Green, FColor::Purple, FColor::Turquoise };

	void AllocateLayerNodes();
	void AllocateLeafNodes();

	void FirstPassRasterize();
	void RasterizeLayer(uint8 aLayer);
	void RasterizeLeafNode(FVector& aOrigin, uint_fast64_t aLeafIndex);

	void Generate();

	TArray<SVONNode>& GetLayer(uint8 aLayer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	bool GetNodePosition(uint8 aLayer, uint_fast64_t aIndex, FVector& oPosition);
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

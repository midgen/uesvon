// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SVONNode.h"
#include "SVONLeafNode.h"
#include "SVONManager.generated.h"

enum class dir : uint8
{
	pX, nX, pY, nY, pZ, nZ
};

typedef uint8 layerindex;
typedef int32 nodeindex;
typedef uint8 subnodeindex;
typedef uint_fast64_t mortoncode;
typedef uint_fast32_t posint;

UCLASS()
class UESVON_API ASVONManager : public AActor
{
	GENERATED_BODY()

public:

	static const uint8 NUM_LAYERS = 6;


	bool GetNodePosition(layerindex aLayer, mortoncode aCode, FVector& oPosition);
	SVONNode& GetNodeAt(layerindex aLayer, posint aX, posint aY, posint aZ);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Sets default values for this actor's properties
	ASVONManager();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
		ASVONBoundsVolume* myBoundsVolume;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
		int32 myVoxelPower;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
		TEnumAsByte<ECollisionChannel> myCollisionChannel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
		bool myShowMortonCodes = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
		bool myShowVoxels = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
		bool myShowLinks = false;

	// Voxel dimensions. 0 is leaf node.
	//float myVoxelSize[NUM_LAYERS];
	//uint32 myLayerSize[NUM_LAYERS];

	FVector myOrigin;
	FVector myExtent;

private:

	TArray<SVONNode> myLayers[NUM_LAYERS];

	TArray<SVONLeafNode> myLeafNodes;
	TSet<mortoncode> myBlockedIndices;

	FColor myLayerColors[7] = { FColor::Orange, FColor::Yellow, FColor::White, FColor::Blue, FColor::Turquoise, FColor::Cyan, FColor::Emerald };

	void AllocateLeafNodes();

	void FirstPassRasterize();
	void RasterizeLayer(layerindex aLayer);
	void BuildNeighbourLinks(layerindex aLayer);
	void RasterizeLeafNode(FVector& aOrigin, nodeindex aLeafIndex);

	bool SetNeighbour(const layerindex aLayer, const nodeindex aArrayIndex, const dir aDirection);

	bool IsAnyMemberBlocked(layerindex aLayer, mortoncode aCode, nodeindex aThisParentIndex, nodeindex& oFirstChildIndex);

	SVONNode& GetNodeFromLink(SVONLink& aLink) ;

	void Generate();

	TArray<SVONNode>& GetLayer(layerindex aLayer);

	float GetVoxelSize(layerindex aLayer);
	int32 GetLayerSize(layerindex aLayer);
	int32 GetNodesInLayer(layerindex aLayer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



};

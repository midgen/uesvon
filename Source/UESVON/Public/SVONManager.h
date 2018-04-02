// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SVONNode.h"
#include "SVONLeafNode.h"
#include "SVONDefines.h"
#include "SVONVolume.h"
#include "UESVON.h"
#include "SVONManager.generated.h"

enum class dir : uint8
{
	pX, nX, pY, nY, pZ, nZ
};

UCLASS()
class UESVON_API ASVONManager : public AActor
{
	GENERATED_BODY()

public:

	bool GetNodePosition(layerindex aLayer, mortoncode aCode, FVector& oPosition);
	SVONNode& GetNodeAt(layerindex aLayer, posint aX, posint aY, posint aZ);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Sets default values for this actor's properties
	ASVONManager();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
		ASVONVolume* myBoundsVolume;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
		int32 myVoxelPower;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
		TEnumAsByte<ECollisionChannel> myCollisionChannel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
		bool myShowMortonCodes = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
		bool myShowVoxels = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
		bool myShowParentChildLinks = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
		bool myShowNeighbourLinks = false;

	FVector myOrigin;
	FVector myExtent;

	uint8 myNumLayers = 0;

private:

	TArray<TArray<SVONNode>> myLayers;

	TArray<SVONLeafNode> myLeafNodes;
	TSet<mortoncode> myBlockedIndices;

	void AllocateLeafNodes();

	void FirstPassRasterize();
	void RasterizeLayer(layerindex aLayer);
	void BuildNeighbourLinks(layerindex aLayer);
	bool FindLinkInDirection(layerindex aLayer, nodeindex aNodeIndex, uint8 aDir, SVONLink& oLinkToUpdate, FVector& aStartPosForDebug);
	void RasterizeLeafNode(FVector& aOrigin, nodeindex aLeafIndex);

	bool SetNeighbour(const layerindex aLayer, const nodeindex aArrayIndex, const dir aDirection);

	bool IsAnyMemberBlocked(layerindex aLayer, mortoncode aCode, nodeindex aThisParentIndex, nodeindex& oFirstChildIndex);

	SVONNode& GetNodeFromLink(SVONLink& aLink) ;

	void Generate();

	TArray<SVONNode>& GetLayer(layerindex aLayer);

	float GetVoxelSize(layerindex aLayer);
	int32 GetNodesInLayer(layerindex aLayer);
	int32 GetNodesPerSide(layerindex aLayer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};

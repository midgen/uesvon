// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SVONNode.h"
#include "SVONLeafNode.h"
#include "SVONDefines.h"
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

	//static const uint8 NUM_LAYERS = 6;

	const FIntVector dirs[6] = {
		FIntVector(1,0,0),
		FIntVector(-1,0,0),
		FIntVector(0,1,0),
		FIntVector(0,-1,0),
		FIntVector(0,0,1),
		FIntVector(0,0,-1)
	};


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

	FColor myLayerColors[8] = { FColor::Orange, FColor::Yellow, FColor::White, FColor::Blue, FColor::Turquoise, FColor::Cyan, FColor::Emerald, FColor::Orange };

	FColor myLinkColors[8] = { FColor(0xFF111111), FColor(0xFFFFFFFF),FColor(0xFF333333), FColor(0xFFDDDDDD), FColor(0xFFBBBBBB), FColor(0xFF999999), FColor(0xFF777777), FColor(0xFF555555)  };

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

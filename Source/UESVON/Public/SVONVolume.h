// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "SVONDefines.h"
#include "SVONNode.h"
#include "SVONLeafNode.h"
#include "UESVON.h"
#include "SVONVolume.generated.h"


/**
 * 
 */
UCLASS(hidecategories = (Tags, Cooking, Collision, Actor, HLOD, Mobile, LOD))
class UESVON_API ASVONVolume : public AVolume
{
	GENERATED_UCLASS_BODY()
	
public:

	//~ Begin AActor Interface
	virtual void PostRegisterAllComponents() override;
	virtual void PostUnregisterAllComponents() override;
	//~ End AActor Interface
#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditUndo() override;
	//~ End UObject Interface
#endif // WITH_EDITOR

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
	bool myShowVoxels = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
	bool myShowMortonCodes = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
	bool myShowNeighbourLinks = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
	int32 myVoxelPower = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
	TEnumAsByte<ECollisionChannel> myCollisionChannel;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
	bool myGenerateOnEdit = false;

	bool Generate();
	
private:

	FVector myOrigin;
	FVector myExtent;

	uint8 myNumLayers = 0;
	
	// SVO data
	TArray<TArray<SVONNode>> myLayers;
	TArray<SVONLeafNode> myLeafNodes;
	// First pass rasterize results
	TSet<mortoncode> myBlockedIndices;

	bool FirstPassRasterize();
	void RasterizeLayer(layerindex aLayer);

	float GetVoxelSize(layerindex aLayer);
	int32 GetNodesInLayer(layerindex aLayer);
	int32 GetNodesPerSide(layerindex aLayer);
	bool GetNodePosition(layerindex aLayer, mortoncode aCode, FVector& oPosition);
	TArray<SVONNode>& GetLayer(layerindex aLayer);

	void BuildNeighbourLinks(layerindex aLayer);
	bool FindLinkInDirection(layerindex aLayer, nodeindex aNodeIndex, uint8 aDir, SVONLink& oLinkToUpdate, FVector& aStartPosForDebug);
	void RasterizeLeafNode(FVector& aOrigin, nodeindex aLeafIndex);
	bool SetNeighbour(const layerindex aLayer, const nodeindex aArrayIndex, const dir aDirection);
	bool IsAnyMemberBlocked(layerindex aLayer, mortoncode aCode, nodeindex aThisParentIndex, nodeindex& oFirstChildIndex);
};

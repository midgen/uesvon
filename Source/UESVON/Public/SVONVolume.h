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
UCLASS(hidecategories = (Tags, Cooking, Actor, HLOD, Mobile, LOD))
class UESVON_API ASVONVolume : public AVolume
{
	GENERATED_UCLASS_BODY()
	
public:

	virtual void BeginPlay() override;

	//~ Begin AActor Interface
	virtual void PostRegisterAllComponents() override;
	virtual void PostUnregisterAllComponents() override;
	//~ End AActor Interface
#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditUndo() override;
	void OnPostShapeChanged();

	bool ShouldTickIfViewportsOnly() const override { return true; }
	//~ End UObject Interface
#endif // WITH_EDITOR

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
	bool myShowVoxels = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
	bool myShowMortonCodes = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
	bool myShowNeighbourLinks = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
	bool myShowParentChildLinks = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
	int32 myVoxelPower = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
	TEnumAsByte<ECollisionChannel> myCollisionChannel;

	bool Generate();

	const FVector& GetOrigin() const { return myOrigin; }
	const FVector& GetExtent() const { return myExtent; }
	const uint8 GetMyNumLayers() const { return myNumLayers; }
	const TArray<SVONNode>& GetLayer(layerindex_t aLayer) const;
	float GetVoxelSize(layerindex_t aLayer) const;

	bool IsReadyForNavigation();
	
	bool GetLinkPosition(const SVONLink& aLink, FVector& oPosition) const;
	bool GetNodePosition(layerindex_t aLayer, mortoncode_t aCode, FVector& oPosition) const;
	const SVONNode& GetNode(const SVONLink& aLink) const;
	const SVONLeafNode& GetLeafNode(nodeindex_t aIndex) const;

	

	
private:
	bool myIsReadyForNavigation = false;

	FVector myOrigin;
	FVector myExtent;

	uint8 myNumLayers = 0;
	
	// SVO data
	TArray<TArray<SVONNode>> myLayers;
	TArray<SVONLeafNode> myLeafNodes;
	// First pass rasterize results
	TArray<TSet<mortoncode_t>> myBlockedIndices;

	TArray<SVONNode>& GetLayer(layerindex_t aLayer);

	bool FirstPassRasterize();
	void RasterizeLayer(layerindex_t aLayer);


	int32 GetNodesInLayer(layerindex_t aLayer);
	int32 GetNodesPerSide(layerindex_t aLayer);


	bool GetIndexForCode(layerindex_t aLayer, mortoncode_t aCode, nodeindex_t& oIndex) const;

	void BuildNeighbourLinks(layerindex_t aLayer);
	bool FindLinkInDirection(layerindex_t aLayer, nodeindex_t aNodeIndex, uint8 aDir, SVONLink& oLinkToUpdate, FVector& aStartPosForDebug);
	void RasterizeLeafNode(FVector& aOrigin, nodeindex_t aLeafIndex);
	bool SetNeighbour(const layerindex_t aLayer, const nodeindex_t aArrayIndex, const dir aDirection);

	bool IsAnyMemberBlocked(layerindex_t aLayer, mortoncode_t aCode);
};

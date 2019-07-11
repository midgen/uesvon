// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "SVONDefines.h"
#include "SVONNode.h"
#include "SVONLeafNode.h"
#include "SVONData.h"
#include "UESVON.h"
#include "SVONVolume.generated.h"


UENUM(BlueprintType)
enum class ESVOGenerationStrategy : uint8
{
	UseBaked UMETA(DisplayName = "Use Baked"),
	GenerateOnBeginPlay UMETA(DisplayName = "Generate OnBeginPlay")
};

/**
 *  SVONVolume contains the navigation data for the volume, and the methods for generating that data
		See SVONMediator for public query functions
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

	// Debug Info
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
	float myDebugDistance = 5000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
	bool myShowVoxels = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
	bool myShowLeafVoxels = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
	bool myShowMortonCodes = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
	bool myShowNeighbourLinks = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
	bool myShowParentChildLinks = false;

	// Generation parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
	int32 myVoxelPower = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
	TEnumAsByte<ECollisionChannel> myCollisionChannel;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
	float myClearance = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
	ESVOGenerationStrategy myGenerationStrategy = ESVOGenerationStrategy::UseBaked;

	// Generated data attributes
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UESVON" )
	uint8 myNumLayers = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UESVON")
	int myNumBytes = 0;

	bool Generate();

	const uint8 GetMyNumLayers() const { return myNumLayers; }
	float GetVoxelSize(layerindex_t aLayer) const;

	bool IsReadyForNavigation();
	
	// Public const getters
	const TArray<SVONNode>& GetLayer(layerindex_t aLayer) const { return myData.myLayers[aLayer]; };
	const SVONNode& GetNode(const SVONLink& aLink) const;
	const SVONLeafNode& GetLeafNode(nodeindex_t aIndex) const;
	
	bool GetLinkPosition(const SVONLink& aLink, FVector& oPosition) const;
	bool GetNodePosition(layerindex_t aLayer, mortoncode_t aCode, FVector& oPosition) const;

	void GetLeafNeighbours(const SVONLink& aLink, TArray<SVONLink>& oNeighbours) const;
	void GetNeighbours(const SVONLink& aLink, TArray<SVONLink>& oNeighbours) const;

	virtual void Serialize(FArchive& Ar) override;

	void ClearData();

private:

	// The navigation data
	SVONData myData;
	// temporary data used during nav data generation first pass rasterize
	TArray<TSet<mortoncode_t>> myBlockedIndices;
	// Helper members
	FVector myOrigin;
	FVector myExtent;
	// Used for defining debug visualiation range
	FVector myDebugPosition;
	bool myIsReadyForNavigation{ false };
	
	TArray<SVONNode>& GetLayer(layerindex_t aLayer) { return myData.myLayers[aLayer]; };

	void UpdateBounds();

	// Generation methods
	bool FirstPassRasterize();
	void RasterizeLayer(layerindex_t aLayer);
	bool GetIndexForCode(layerindex_t aLayer, mortoncode_t aCode, nodeindex_t& oIndex) const;
	void BuildNeighbourLinks(layerindex_t aLayer);
	bool FindLinkInDirection(layerindex_t aLayer, const nodeindex_t aNodeIndex, uint8 aDir, SVONLink& oLinkToUpdate, FVector& aStartPosForDebug);
	void RasterizeLeafNode(FVector& aOrigin, nodeindex_t aLeafIndex);
	bool IsAnyMemberBlocked(layerindex_t aLayer, mortoncode_t aCode) const;
	bool IsBlocked(const FVector& aPosition, const float aSize) const;	

	// Getters
	int32 GetNumNodesInLayer(layerindex_t aLayer) const;
	int32 GetNumNodesPerSide(layerindex_t aLayer) const;

	// Debug methods
	bool IsInDebugRange(const FVector& aPosition) const;
};

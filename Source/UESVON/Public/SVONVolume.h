#pragma once

#include "UESVON/Public/SVONData.h"
#include "UESVON/Public/SVONDefines.h"
#include "UESVON/Public/SVONLeafNode.h"
#include "UESVON/Public/SVONNode.h"
#include "UESVON/Public/UESVON.h"

#include <Runtime/Engine/Classes/GameFramework/Volume.h>

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
	GENERATED_BODY()

public:

	ASVONVolume(const FObjectInitializer& ObjectInitializer);

	//~ Begin AActor Interface
	void BeginPlay() override;
	void PostRegisterAllComponents() override;
	void PostUnregisterAllComponents() override;
	bool ShouldTickIfViewportsOnly() const override { return true; }
	//~ End AActor Interface

#if WITH_EDITOR
	//~ Begin UObject Interface
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	void PostEditUndo() override;
	//~ End UObject Interface
#endif // WITH_EDITOR 

	//~ Begin UObject 
	void Serialize(FArchive& Ar) override;
	//~ End UObject 

	bool Generate();
	void ClearData();

	bool IsReadyForNavigation() const;
	const TArray<SVONNode>& GetLayer(layerindex_t aLayer) const { return myData.myLayers[aLayer]; };
	const SVONNode& GetNode(const SVONLink& aLink) const;
	const SVONLeafNode& GetLeafNode(nodeindex_t aIndex) const;
	bool GetLinkPosition(const SVONLink& aLink, FVector& oPosition) const;
	bool GetNodePosition(layerindex_t aLayer, mortoncode_t aCode, FVector& oPosition) const;
	void GetLeafNeighbours(const SVONLink& aLink, TArray<SVONLink>& oNeighbours) const;
	void GetNeighbours(const SVONLink& aLink, TArray<SVONLink>& oNeighbours) const;
	float GetVoxelSize(layerindex_t aLayer) const;
	const uint8 GetMyNumLayers() const { return myNumLayers; }

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UESVON")
	uint8 myNumLayers = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UESVON")
	int myNumBytes = 0;

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

	TArray<SVONNode>& GetLayer(layerindex_t aLayer) { return myData.myLayers[aLayer]; };

	bool myIsReadyForNavigation;

	void UpdateBounds();

	// Generation methods
	bool FirstPassRasterize();
	void RasterizeLayer(layerindex_t aLayer);
	void BuildNeighbourLinks(layerindex_t aLayer);
	bool FindLinkInDirection(layerindex_t aLayer, const nodeindex_t aNodeIndex, uint8 aDir, SVONLink& oLinkToUpdate, FVector& aStartPosForDebug);
	void RasterizeLeafNode(FVector& aOrigin, nodeindex_t aLeafIndex);

	bool GetIndexForCode(layerindex_t aLayer, mortoncode_t aCode, nodeindex_t& oIndex) const;
	bool IsAnyMemberBlocked(layerindex_t aLayer, mortoncode_t aCode) const;
	bool IsBlocked(const FVector& aPosition, const float aSize) const;
	int32 GetNumNodesInLayer(layerindex_t aLayer) const;
	int32 GetNumNodesPerSide(layerindex_t aLayer) const;

	bool IsInDebugRange(const FVector& aPosition) const;
};

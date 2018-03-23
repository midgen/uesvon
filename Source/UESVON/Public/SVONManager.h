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


	bool GetNodePosition(uint8 aLayer, uint_fast64_t aCode, FVector& oPosition);
	SVONNode& GetNodeAt(uint8 aLayer, uint_fast32_t aX, uint_fast32_t aY, uint_fast32_t aZ);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Sets default values for this actor's properties
	ASVONManager();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
		ASVONBoundsVolume* myBoundsVolume;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
		uint8 myVoxelPower;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
		TEnumAsByte<ECollisionChannel> myCollisionChannel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
		bool myShowMortonCodes = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UESVON")
		bool myShowVoxels = false;

	// Voxel dimensions. 0 is leaf node.
	float myVoxelSize[NUM_LAYERS];
	uint32 myLayerSize[NUM_LAYERS];

	FVector myOrigin;
	FVector myExtent;


private:

	

	TArray<SVONNode> myLayers[NUM_LAYERS];

	TArray<SVONLeafNode> myLeafNodes;
	TSet<uint_fast64_t> myBlockedIndices;

	FColor myLayerColors[5] = { FColor::Orange, FColor::Yellow, FColor::White, FColor::Blue, FColor::Turquoise };
	FColor myLinkColors[5] = { FColor::Cyan, FColor::Blue, FColor::Turquoise, FColor::Emerald, FColor::White};

	void AllocateLayerNodes();
	void AllocateLeafNodes();

	void FirstPassRasterize();
	void RasterizeLayer(uint8 aLayer);
	void BuildNeighbourLinks(uint8 aLayer);
	void RasterizeLeafNode(FVector& aOrigin, uint_fast64_t aLeafIndex);

	bool IsAnyMemberBlocked(uint8 aLayer, uint_fast64_t aCode);

	SVONNode& GetNodeFromLink(SVONLink& aLink) ;

	void Generate();

	TArray<SVONNode>& GetLayer(uint8 aLayer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SVONNode.h"
#include "SVONManager.generated.h"

UCLASS()
class UESVON_API ASVONManager : public AActor
{
	GENERATED_BODY()

private:
	TArray<SVONNode> myNodes;


	void Allocate(const int32 x, const int32 y, const int32 z);

	void Generate();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	SVONNode& GetNodeAt(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z);

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

		float myVoxelSize;

	FVector myOrigin;
	FVector myExtent;
};

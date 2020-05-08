// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UESVON/Public/SVONLink.h"
#include "UESVON/Public/SVONNavigationPath.h"
#include "UESVON/Public/SVONTypes.h"

#include <Runtime/Engine/Classes/Components/ActorComponent.h>

#include "SVONNavigationComponent.generated.h"

class ASVONVolume;
struct SVONLink;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UESVON_API USVONNavigationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation | Debug")
	bool DebugPrintCurrentPosition;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation | Debug")
	bool DebugPrintMortonCodes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation | Debug")
	bool DebugDrawOpenNodes = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation | Heuristics")
	bool UseUnitCost = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation | Heuristics")
	float UnitCost = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation | Heuristics")
	float EstimateWeight = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation | Heuristics")
	float NodeSizeCompensation = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation | Heuristics")
	ESVONPathCostType PathCostType = ESVONPathCostType::EUCLIDEAN;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation | Smoothing")
	int SmoothingIterations = 0;

	// Sets default values for this component's properties
	USVONNavigationComponent(const FObjectInitializer& ObjectInitializer);

public:
	const ASVONVolume* GetCurrentVolume() const { return myCurrentNavVolume; }

	// Get a Nav position
	SVONLink GetNavPosition(FVector& aPosition) const;
	virtual FVector GetPawnPosition() const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/* This method isn't hooked up at the moment, pending integration with existing systems */
	bool FindPathAsync(const FVector& aStartPosition, const FVector& aTargetPosition, FThreadSafeBool& aCompleteFlag, FSVONNavPathSharedPtr* oNavPath);
	bool FindPathImmediate(const FVector& aStartPosition, const FVector& aTargetPosition, FSVONNavPathSharedPtr* oNavPath);

	UFUNCTION(BlueprintCallable, Category = UESVON)
	void FindPathImmediate(const FVector &aStartPosition, const FVector &aTargetPosition, TArray<FVector>& OutPathPoints);

	FSVONNavPathSharedPtr& GetPath() { return mySVONPath; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// The current navigation volume
	ASVONVolume* myCurrentNavVolume;
	bool HasNavData() const;

	// Check the scene for a valid volume that I am within the extents of
	bool FindVolume();

	// Print current layer/morton code information
	void DebugLocalPosition(FVector& aPosition);

	FSVONNavPathSharedPtr mySVONPath;

	mutable SVONLink myLastLocation;
};

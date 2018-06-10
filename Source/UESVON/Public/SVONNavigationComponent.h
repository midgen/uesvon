// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SVONPath.h"
#include "SVONLink.h"
#include "SVONNavigationComponent.generated.h"

class ASVONVolume;
struct SVONLink;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UESVON_API USVONNavigationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation")
	bool DebugPrintCurrentPosition;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation")
	bool DebugPrintMortonCodes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation")
	bool DebugPathToOverride = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation")
	FVector DebugPathTo;
	// Sets default values for this component's properties
	USVONNavigationComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// The current navigation volume
	ASVONVolume* myCurrentNavVolume;

	// Do I have a valid nav volume ready?
	bool HasNavVolume();

	// Check the scene for a valid volume that I am within the extents of
	bool FindVolume();

	// Print current layer/morton code information
	void DebugLocalPosition(FVector& aPosition);

	SVONPath myCurrentPath;

	SVONLink myLastLocation;

	TQueue<int> myJobQueue;

	bool myIsBusy;

public:	
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Get a Nav position
	SVONLink GetNavPosition(FVector& aPosition);

	bool FindPath(FVector& aTargetPosition);

};

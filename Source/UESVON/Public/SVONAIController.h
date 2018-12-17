// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "SVONNavigationComponent.h"
#include "UESVON.h"
#include "SVONAIController.generated.h"


/**
 * 
 */
UCLASS()
class UESVON_API ASVONAIController : public AAIController
{
	GENERATED_BODY()

	ASVONAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	FNavPathSharedPtr myNavPath;

	/** Component used for moving along a path. */
	UPROPERTY(VisibleDefaultsOnly, Category = SVON)
	USVONNavigationComponent* SVONNavComponent;

	FPathFollowingRequestResult MoveTo(const FAIMoveRequest& MoveRequest, FNavPathSharedPtr* OutPath = nullptr) override;
	
};

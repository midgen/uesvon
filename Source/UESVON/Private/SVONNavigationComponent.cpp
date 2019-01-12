// Fill out your copyright notice in the Description page of Project Settings.

#include "SVONNavigationComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "SVONVolume.h"
#include "SVONLink.h"
#include "SVONPathFinder.h"
#include "SVONNavigationPath.h"
#include "SVONFindPathTask.h"
#include "DrawDebugHelpers.h"
#include "Runtime/NavigationSystem/Public/NavigationData.h"
#include "Runtime/Engine/Classes/Components/LineBatchComponent.h"

// Sets default values for this component's properties
USVONNavigationComponent::USVONNavigationComponent()
	: myIsBusy(false)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	myLastLocation = SVONLink(0, 0, 0);

	mySVONPath = MakeShareable<FSVONNavigationPath>(new FSVONNavigationPath());

}


// Called when the game starts
void USVONNavigationComponent::BeginPlay()
{
	Super::BeginPlay();
}

/** Are we inside a valid nav volume ? */
bool USVONNavigationComponent::HasNavVolume()
{
	return myCurrentNavVolume && GetOwner() && myCurrentNavVolume->EncompassesPoint(GetPawnPosition()) && myCurrentNavVolume->GetMyNumLayers() > 0;
}

bool USVONNavigationComponent::FindVolume()
{
	TArray<AActor*> navVolumes;

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASVONVolume::StaticClass(), navVolumes);

	for (AActor* actor : navVolumes)
	{
		ASVONVolume* volume = Cast<ASVONVolume>(actor);
		if (volume && volume->EncompassesPoint(GetPawnPosition()))
		{
			myCurrentNavVolume = volume;
			return true;
		}
	}
	return false;
}

// Called every frame
void USVONNavigationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	//return;

	if (!HasNavVolume())
	{
		FindVolume();
	}
	else if (myCurrentNavVolume->IsReadyForNavigation() && !myIsBusy)
	{
		FVector location = GetPawnPosition();
		if (DebugPrintMortonCodes)
		{
			DebugLocalPosition(location);
		}
		SVONLink link = GetNavPosition(location);
	}

	int q;
	if (myJobQueue.Dequeue(q))
	{
		if (q > 0)
		{
			myPointDebugIndex = 0;
		}
		else
		{
			myIsBusy = false;
		}
	}

	if (myIsBusy && myPointDebugIndex > -1)
	{
		if (DebugDrawOpenNodes)
		{
			if (myDebugPoints.Num() > 0)
			{
				DrawDebugSphere(GetWorld(), myDebugPoints[myPointDebugIndex], 100.f, 5, FColor::Red, true);
				DrawDebugString(GetWorld(), myDebugPoints[myPointDebugIndex], FString::FromInt(myPointDebugIndex));
			}
			if (myPointDebugIndex < myDebugPoints.Num() - 1)
			{
				myPointDebugIndex++;
			}
			else
			{
				myIsBusy = false;
				myPointDebugIndex = -1;
			}
		}
		else
		{
			myIsBusy = false;
			myPointDebugIndex = -1;
		}




	}




}
SVONLink USVONNavigationComponent::GetNavPosition(FVector& aPosition)
{
	SVONLink navLink;
	if (HasNavVolume())
	{
		// Get the nav link from our volume
		SVONMediator::GetLinkFromPosition(GetOwner()->GetActorLocation(), *myCurrentNavVolume, navLink);

		if (navLink == myLastLocation)
			return navLink;

		myLastLocation = navLink;

		//FVector targetPos = GetPawnPosition() + (GetOwner()->GetActorForwardVector() * 10000.f);

		if (DebugPrintCurrentPosition)
		{
			const SVONNode& currentNode = myCurrentNavVolume->GetNode(navLink);
			FVector currentNodePosition;

			bool isValid = myCurrentNavVolume->GetLinkPosition(navLink, currentNodePosition);

			DrawDebugLine(GetWorld(), GetPawnPosition(), currentNodePosition, isValid ? FColor::Green : FColor::Red, false, -1.f, 0, 10.f);
			DrawDebugString(GetWorld(), GetPawnPosition() + FVector(0.f, 0.f, -50.f), navLink.ToString(), NULL, FColor::Yellow, 0.01f);
		}
		
	}
	return navLink;
}

bool USVONNavigationComponent::FindPathAsync(const FVector& aStartPosition, const FVector& aTargetPosition, FThreadSafeBool& aCompleteFlag, FSVONNavPathSharedPtr* oNavPath)
{
#if WITH_EDITOR
	UE_LOG(UESVON, Log, TEXT("Finding path from %s and %s"), *aStartPosition.ToString(), *aTargetPosition.ToString());
#endif
	SVONLink startNavLink;
	SVONLink targetNavLink;
	if (HasNavVolume())
	{
		// Get the nav link from our volume
		if (!SVONMediator::GetLinkFromPosition(aStartPosition, *myCurrentNavVolume, startNavLink))
		{
#if WITH_EDITOR
			UE_LOG(UESVON, Error, TEXT("Path finder failed to find start nav link. Is your pawn blocking the channel you've selected to generate the nav data with?"));
#endif
			return false;
		}

		if (!SVONMediator::GetLinkFromPosition(aTargetPosition, *myCurrentNavVolume, targetNavLink))
		{
#if WITH_EDITOR
			UE_LOG(UESVON, Error, TEXT("Path finder failed to find target nav link"));
#endif
			return false;
		}

		myDebugPoints.Empty();
		myPointDebugIndex = -1;

		SVONPathFinderSettings settings;
		settings.myUseUnitCost = UseUnitCost;
		settings.myUnitCost = UnitCost;
		settings.myEstimateWeight = EstimateWeight;
		settings.myNodeSizeCompensation = NodeSizeCompensation;
		settings.myPathCostType = PathCostType;
		settings.mySmoothingIterations = SmoothingIterations;

		(new FAutoDeleteAsyncTask<FSVONFindPathTask>(*myCurrentNavVolume, settings, GetWorld(), startNavLink, targetNavLink, aStartPosition, aTargetPosition, oNavPath, aCompleteFlag, myDebugPoints  ))->StartBackgroundTask();

		myIsBusy = true;

		return true;


	}
	else
	{
#if WITH_EDITOR
		UE_LOG(UESVON, Error, TEXT("Pawn is not inside an SVON volume, or nav data has not been generated"));
#endif
	}

	return false;
}

bool USVONNavigationComponent::FindPathImmediate(const FVector& aStartPosition, const FVector& aTargetPosition, FSVONNavPathSharedPtr* oNavPath)
{
#if WITH_EDITOR
	UE_LOG(UESVON, Log, TEXT("Finding path immediate from %s and %s"), *aStartPosition.ToString(), *aTargetPosition.ToString());
#endif

	SVONLink startNavLink;
	SVONLink targetNavLink;
	if (HasNavVolume())
	{
		// Get the nav link from our volume
		if (!SVONMediator::GetLinkFromPosition(aStartPosition, *myCurrentNavVolume, startNavLink))
		{
#if WITH_EDITOR
			UE_LOG(UESVON, Error, TEXT("Path finder failed to find start nav link"));
#endif
			return false;
		}

		if (!SVONMediator::GetLinkFromPosition(aTargetPosition, *myCurrentNavVolume, targetNavLink))
		{
#if WITH_EDITOR
			UE_LOG(UESVON, Error, TEXT("Path finder failed to find target nav link"));
#endif
			return false;
		}

		if (!oNavPath || !oNavPath->IsValid())
		{
#if WITH_EDITOR
			UE_LOG(UESVON, Error, TEXT("Nav path data invalid"));
#endif
			return false;
		}

		//FNavigationPath* path = oNavPath->Get();

		FSVONNavigationPath* path = oNavPath->Get();

		path->ResetForRepath();

		myDebugPoints.Empty();
		myPointDebugIndex = -1;

		TArray<FVector> debugOpenPoints;

		SVONPathFinderSettings settings;
		settings.myUseUnitCost = UseUnitCost;
		settings.myUnitCost = UnitCost;
		settings.myEstimateWeight = EstimateWeight;
		settings.myNodeSizeCompensation = NodeSizeCompensation;
		settings.myPathCostType = PathCostType;
		settings.mySmoothingIterations = SmoothingIterations;

		SVONPathFinder pathFinder(GetWorld(), *myCurrentNavVolume, settings);

		int result = pathFinder.FindPath(startNavLink, targetNavLink, aStartPosition, aTargetPosition, oNavPath);

		myIsBusy = true;
		myPointDebugIndex = 0;


		path->SetIsReady(true);

		return true;

	}
	else
	{
#if WITH_EDITOR
		UE_LOG(UESVON, Error, TEXT("Pawn is not inside an SVON volume, or nav data has not been generated"));
#endif
	}

	return false;
}

void USVONNavigationComponent::DebugLocalPosition(FVector& aPosition) 
{

	if (HasNavVolume())
	{
		for (int i = 0; i < myCurrentNavVolume->GetMyNumLayers() - 1; i++)
		{
			FIntVector pos;
			SVONMediator::GetVolumeXYZ(GetPawnPosition(), *myCurrentNavVolume, i,  pos);
			uint_fast64_t code = morton3D_64_encode(pos.X, pos.Y, pos.Z);
			FString codeString = FString::FromInt(code);
			DrawDebugString(GetWorld(), GetPawnPosition() + FVector(0.f, 0.f, i*50.0f), pos.ToString() + " - " + codeString, NULL, FColor::White, 0.01f);
		}
	

	}

}
 

FVector USVONNavigationComponent::GetPawnPosition()
{
	FVector result;

	AController * controller = Cast<AController>(GetOwner());

	if (controller)
	{
		if (APawn* pawn = controller->GetPawn())
			result = pawn->GetActorLocation();
	}

	return result;

}
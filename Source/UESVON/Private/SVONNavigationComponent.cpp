// Fill out your copyright notice in the Description page of Project Settings.

#include "SVONNavigationComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "SVONVolume.h"
#include "SVONLink.h"
#include "SVONPathFinder.h"
#include "SVONPath.h"
#include "SVONFindPathTask.h"
#include "DrawDebugHelpers.h"
#include "Runtime/Engine/Classes/Components/LineBatchComponent.h "

// Sets default values for this component's properties
USVONNavigationComponent::USVONNavigationComponent()
	: myIsBusy(false)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	myLastLocation = SVONLink(0, 0, 0);

	// ...
}


// Called when the game starts
void USVONNavigationComponent::BeginPlay()
{
	Super::BeginPlay();
}

/** Are we inside a valid nav volume ? */
bool USVONNavigationComponent::HasNavVolume()
{
	return myCurrentNavVolume && GetOwner() && myCurrentNavVolume->EncompassesPoint(GetOwner()->GetActorLocation());
}

bool USVONNavigationComponent::FindVolume()
{
	TArray<AActor*> navVolumes;

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASVONVolume::StaticClass(), navVolumes);

	for (AActor* actor : navVolumes)
	{
		ASVONVolume* volume = Cast<ASVONVolume>(actor);
		if (volume && volume->EncompassesPoint(GetOwner()->GetActorLocation()))
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

	if (!HasNavVolume())
	{
		FindVolume();
	}
	else if (myCurrentNavVolume->IsReadyForNavigation() && !myIsBusy)
	{
		FVector location = GetOwner()->GetActorLocation();
		if (DebugPrintMortonCodes)
		{
			DebugLocalPosition(location);
		}
		SVONLink link = GetNavPosition(location);
	}

	int q;
	if (myJobQueue.Dequeue(q))
	{
		GetWorld()->PersistentLineBatcher->Flush();
		if (q > 0)
		{
			myCurrentPath.AddPoint(GetOwner()->GetActorLocation());
			myCurrentPath.DebugDraw(GetWorld());
			myPointDebugIndex = 0;
		}
		else
		{
			myIsBusy = false;
		}
	}

	if (myIsBusy && myPointDebugIndex > -1)
	{
		if(myDebugPoints.Num() > 0)
			DrawDebugSphere(GetWorld(), myDebugPoints[myPointDebugIndex], 100.f, 5, FColor::Red, true);

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

		FVector targetPos = GetOwner()->GetActorLocation() + (GetOwner()->GetActorForwardVector() * 10000.f);
		FindPath(targetPos);

		if (DebugPrintCurrentPosition)
		{
			const SVONNode& currentNode = myCurrentNavVolume->GetNode(navLink);
			FVector currentNodePosition;

			bool isValid = myCurrentNavVolume->GetLinkPosition(navLink, currentNodePosition);

			DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(), currentNodePosition, isValid ? FColor::Green : FColor::Red, false, -1.f, 0, 10.f);
			DrawDebugString(GetWorld(), GetOwner()->GetActorLocation() + FVector(0.f, 0.f, -50.f), navLink.ToString(), NULL, FColor::Yellow, 0.01f);
		}
		
	}
	return navLink;
}

bool USVONNavigationComponent::FindPath(FVector& aTargetPosition)
{
	UE_LOG(UESVON, Display, TEXT("Finding path from %s and %s"), *GetOwner()->GetActorLocation().ToString(), *aTargetPosition.ToString());

	if (DebugPathToOverride)
		aTargetPosition = DebugPathTo;

	SVONLink startNavLink;
	SVONLink targetNavLink;
	if (HasNavVolume())
	{
		// Get the nav link from our volume
		if (!SVONMediator::GetLinkFromPosition(GetOwner()->GetActorLocation(), *myCurrentNavVolume, startNavLink))
		{
			UE_LOG(UESVON, Display, TEXT("Path finder failed to find start nav link"));
			return false;
		}



		if (!SVONMediator::GetLinkFromPosition(aTargetPosition, *myCurrentNavVolume, targetNavLink))
		{
			UE_LOG(UESVON, Display, TEXT("Path finder failed to find target nav link"));
			return false;
		}

		// Reset the path and debug container
		myCurrentPath.ResetPath();

		myDebugPoints.Empty();
		myPointDebugIndex = -1;

		myCurrentPath.AddPoint(aTargetPosition);

		(new FAutoDeleteAsyncTask<FSVONFindPathTask>(*myCurrentNavVolume, GetWorld(), startNavLink, targetNavLink, myCurrentPath, myJobQueue, myDebugPoints))->StartBackgroundTask();

		myIsBusy = true;

		return true;


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
			SVONMediator::GetVolumeXYZ(GetOwner()->GetActorLocation(), *myCurrentNavVolume, i,  pos);
			uint_fast64_t code = morton3D_64_encode(pos.X, pos.Y, pos.Z);
			FString codeString = FString::FromInt(code);
			DrawDebugString(GetWorld(), GetOwner()->GetActorLocation() + FVector(0.f, 0.f, i*50.0f), pos.ToString() + " - " + codeString, NULL, FColor::White, 0.01f);
		}
	

	}

}
 

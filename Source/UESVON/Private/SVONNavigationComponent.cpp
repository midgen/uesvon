// Fill out your copyright notice in the Description page of Project Settings.

#include "SVONNavigationComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "SVONVolume.h"
#include "SVONLink.h"
#include "SVONPathFinder.h"
#include "SVONPath.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
USVONNavigationComponent::USVONNavigationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

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
	else if (myCurrentNavVolume->IsReadyForNavigation())
	{
		FVector location = GetOwner()->GetActorLocation();
		if (DebugPrintMortonCodes)
		{
			DebugLocalPosition(location);
		}
		SVONLink link = GetNavPosition(location);
	}



}
SVONLink USVONNavigationComponent::GetNavPosition(FVector& aPosition)
{
	SVONLink navLink;
	if (HasNavVolume())
	{
		// Get the nav link from our volume
		SVONMediator::GetLinkFromPosition(GetOwner()->GetActorLocation(), *myCurrentNavVolume, navLink);
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

		SVONPath newPath;
		newPath.AddPoint(aTargetPosition);
		
		SVONPathFinder pathFinder(*myCurrentNavVolume, true, GetWorld());

		//DrawDebugSphere(GetWorld(), aTargetPosition, 200.0f, 30, FColor::Cyan, false);
		//DrawDebugString(GetWorld(), aTargetPosition + FVector(0.f, 0.f, 50.f), targetNavLink.ToString());

		pathFinder.FindPath(startNavLink, targetNavLink, newPath);

		newPath.AddPoint(GetOwner()->GetActorLocation());

		newPath.DebugDraw(GetWorld());

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
 

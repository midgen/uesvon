// Fill out your copyright notice in the Description page of Project Settings.

#include "SVONNavigationComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "SVONVolume.h"
#include "SVONLink.h"
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
		
		DebugLocalPosition(location);
		//SVONLink link = GetNavPosition(location);
	}


	// ...
}

SVONLink USVONNavigationComponent::GetNavPosition(FVector& aPosition)
{
	SVONLink navLink;
	if (HasNavVolume())
	{
		
		SVONMediator::GetLinkFromPosition(GetOwner()->GetActorLocation(), *myCurrentNavVolume, navLink);
		//DrawDebugString(GetWorld(), GetOwner()->GetActorLocation(), navLink.ToString());
		
	}
	return navLink;
}

void USVONNavigationComponent::DebugLocalPosition(FVector& aPosition)
{

	if (HasNavVolume())
	{
		for (int i = 0; i < myCurrentNavVolume->GetMyNumLayers(); i++)
		{
			FIntVector pos;
			SVONMediator::GetVolumeXYZ(GetOwner()->GetActorLocation(), *myCurrentNavVolume, i,  pos);
			uint_fast64_t code = morton3D_64_encode(pos.X, pos.Y, pos.Z);
			FString codeString = FString::FromInt(code);
			DrawDebugString(GetWorld(), GetOwner()->GetActorLocation() + FVector(0.f, 0.f, i*50.0f), pos.ToString() + " - " + codeString);
		}
	

	}

}


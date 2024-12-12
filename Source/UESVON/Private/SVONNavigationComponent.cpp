
#include <UESVON/Public/SVONNavigationComponent.h>

#include <UESVON/Public/Subsystem/SVONSubsystem.h>
#include <UESVON/Public/SVONFindPathTask.h>
#include <UESVON/Public/SVONLink.h>
#include <UESVON/Public/SVONNavigationPath.h>
#include <UESVON/Public/SVONPathFinder.h>
#include <UESVON/Public/SVONVolume.h>
#include <UESVON/Public/SVONMediator.h>
#include <UESVON/Public/UESVON.h>

#include <Runtime/Engine/Classes/Components/LineBatchComponent.h>
#include <Runtime/Engine/Classes/GameFramework/Actor.h>
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>
#include <Runtime/Engine/Public/DrawDebugHelpers.h>
#include <Runtime/NavigationSystem/Public/NavigationData.h>

// Sets default values for this component's properties
USVONNavigationComponent::USVONNavigationComponent(const FObjectInitializer& ObjectInitializer)
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

	if (USVONSubsystem* SvonSubsystem = GetWorld()->GetSubsystem<USVONSubsystem>())
	{
		SvonSubsystem->RegisterNavComponent(this);
		myCurrentNavVolume = SvonSubsystem->GetVolumeForPosition(GetPawnPosition());
	}
}

void USVONNavigationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (USVONSubsystem* SvonSubsystem = GetWorld()->GetSubsystem<USVONSubsystem>())
	{
		SvonSubsystem->UnRegisterNavComponent(this);
	}

	Super::EndPlay(EndPlayReason);
}

bool USVONNavigationComponent::HasNavData() const
{
	return myCurrentNavVolume && myCurrentNavVolume;
}

bool USVONNavigationComponent::FindVolume()
{
	TArray<AActor*> navVolumes;

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASVONVolume::StaticClass(), navVolumes);

	for (AActor* actor : navVolumes)
	{
		ASVONVolume* volume = Cast<ASVONVolume>(actor);
		float DistanceToPoint = 0.f;
		if (volume && volume->EncompassesPoint(GetPawnPosition(), 0.5f, &DistanceToPoint))
		{
			myCurrentNavVolume = volume;
			return true;
		}
	}
	return false;
}

SVONLink USVONNavigationComponent::GetNavPosition() const
{
	SVONLink navLink;
	if (HasNavData())
	{
		// Get the nav link from our volume
		SVONMediator::GetLinkFromPosition(GetOwner()->GetActorLocation(), *myCurrentNavVolume, navLink);

		if (navLink == myLastLocation)
			return navLink;

		myLastLocation = navLink;

		if (DebugPrintCurrentPosition)
		{
			const SVONNode& currentNode = myCurrentNavVolume->GetNavData().OctreeData.GetNode(navLink);
			FVector currentNodePosition;

			bool isValid = myCurrentNavVolume->GetNavData().GetLinkPosition(navLink, currentNodePosition);

			DrawDebugLine(GetWorld(), GetPawnPosition(), currentNodePosition, isValid ? FColor::Green : FColor::Red, false, -1.f, 0, 10.f);
			DrawDebugString(GetWorld(), GetPawnPosition() + FVector(0.f, 0.f, -50.f), navLink.ToString(), NULL, FColor::Yellow, 0.01f);
		}
	}
	return navLink;
}

bool USVONNavigationComponent::FindPathAsync(const FVector& StartPosition, const FVector& TargetPosition, FThreadSafeBool& CompleteFlag, FSVONNavPathSharedPtr* OutNavPath)
{
#if WITH_EDITOR
	UE_LOG(UESVON, Log, TEXT("Finding path from %s and %s"), *StartPosition.ToString(), *TargetPosition.ToString());
#endif
	SVONLink StartNavLink;
	SVONLink TargetNavLink;
	if (HasNavData())
	{
		// Get the nav link from our volume
		if (!SVONMediator::GetLinkFromPosition(StartPosition, *myCurrentNavVolume, StartNavLink))
		{
#if WITH_EDITOR
			UE_LOG(UESVON, Error, TEXT("Path finder failed to find start nav link. Is your pawn blocking the channel you've selected to generate the nav data with?"));
#endif
			return false;
		}

		if (!SVONMediator::GetLinkFromPosition(TargetPosition, *myCurrentNavVolume, TargetNavLink))
		{
#if WITH_EDITOR
			UE_LOG(UESVON, Error, TEXT("Path finder failed to find target nav link"));
#endif
			return false;
		}

		SVONPathFinderSettings Settings;
		Settings.myUseUnitCost = UseUnitCost;
		Settings.myUnitCost = UnitCost;
		Settings.myEstimateWeight = EstimateWeight;
		Settings.myNodeSizeCompensation = NodeSizeCompensation;
		Settings.myPathCostType = PathCostType;
		Settings.mySmoothingIterations = SmoothingIterations;

		(new FAutoDeleteAsyncTask<FSVONFindPathTask>(myCurrentNavVolume->GetNavData(), Settings, GetWorld(), StartNavLink, TargetNavLink, StartPosition, TargetPosition, OutNavPath, CompleteFlag))->StartBackgroundTask();

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
	if (HasNavData())
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

		FSVONNavigationPath* path = oNavPath->Get();

		path->ResetForRepath();

		SVONPathFinderSettings Settings;
		Settings.myUseUnitCost = UseUnitCost;
		Settings.myUnitCost = UnitCost;
		Settings.myEstimateWeight = EstimateWeight;
		Settings.myNodeSizeCompensation = NodeSizeCompensation;
		Settings.myPathCostType = PathCostType;
		Settings.mySmoothingIterations = SmoothingIterations;

		SVONPathFinder pathFinder(GetWorld(), myCurrentNavVolume->GetNavData(), Settings);

		int32 Result = pathFinder.FindPath(startNavLink, targetNavLink, aStartPosition, aTargetPosition, oNavPath);

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

void USVONNavigationComponent::FindPathImmediate(const FVector& aStartPosition, const FVector& aTargetPosition, TArray<FVector>& OutPathPoints)
{
	FindPathImmediate(aStartPosition, aTargetPosition, &mySVONPath);

	OutPathPoints.Empty();

	for (const FSVONPathPoint& PathPoint : mySVONPath->GetPathPoints())
	{
		OutPathPoints.Emplace(PathPoint.myPosition);
	}
}

void USVONNavigationComponent::SetCurrentNavVolume(const ASVONVolume* Volume)
{
	myCurrentNavVolume = Volume;

	if (myCurrentNavVolume && myCurrentNavVolume->bIsReadyForNavigation)
	{
		FVector Location = GetPawnPosition();
		if (DebugPrintMortonCodes)
		{
			DebugLocalPosition(Location);
		}
	}
}

void USVONNavigationComponent::DebugLocalPosition(FVector& aPosition)
{
	if (HasNavData())
	{
		for (int i = 0; i < myCurrentNavVolume->GetNavData().OctreeData.GetNumLayers() - 1; i++)
		{
			FIntVector pos;
			SVONMediator::GetVolumeXYZ(GetPawnPosition(), *myCurrentNavVolume, i, pos);
			uint_fast64_t code = morton3D_64_encode(pos.X, pos.Y, pos.Z);
			FString codeString = FString::FromInt(code);
			DrawDebugString(GetWorld(), GetPawnPosition() + FVector(0.f, 0.f, i * 50.0f), pos.ToString() + " - " + codeString, NULL, FColor::White, 0.01f);
		}
	}
}

FVector USVONNavigationComponent::GetPawnPosition() const
{
	FVector result;

	AController* controller = Cast<AController>(GetOwner());

	if (controller)
	{
		if (APawn* pawn = controller->GetPawn())
			result = pawn->GetActorLocation();
	}

	return result;
}
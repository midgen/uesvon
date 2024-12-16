
#include <UESVON/Public/Component/SVONNavigationComponent.h>

#include <UESVON/Public/Subsystem/SVONSubsystem.h>
#include <UESVON/Public/Task/SVONFindPathTask.h>
#include <UESVON/Public/Data/SVONLink.h>
#include <UESVON/Public/Pathfinding/SVONNavigationPath.h>
#include <UESVON/Public/Pathfinding/SVONPathFinder.h>
#include <UESVON/Public/Actor/SVONVolume.h>
#include <UESVON/Public/Util/SVONMediator.h>
#include <UESVON/Public/UESVON.h>

#include <Runtime/Engine/Classes/Components/LineBatchComponent.h>
#include <Runtime/Engine/Classes/GameFramework/Actor.h>
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>
#include <Runtime/Engine/Public/DrawDebugHelpers.h>
#include <Runtime/NavigationSystem/Public/NavigationData.h>

USVONNavigationComponent::USVONNavigationComponent(const FObjectInitializer& ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentPath = MakeShareable<FSVONNavigationPath>(new FSVONNavigationPath());
}

void USVONNavigationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (USVONSubsystem* SvonSubsystem = GetWorld()->GetSubsystem<USVONSubsystem>())
	{
		SvonSubsystem->RegisterNavComponent(this);
		CurrentNavVolume = SvonSubsystem->GetVolumeForPosition(GetPawnPosition());
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
	return CurrentNavVolume != nullptr;
}

SVONLink USVONNavigationComponent::GetNavPosition() const
{
	SVONLink NavLink;
	if (HasNavData())
	{
		SVONMediator::GetLinkFromPosition(GetOwner()->GetActorLocation(), *CurrentNavVolume, NavLink);

		if (bDebugPrintCurrentPosition)
		{
			const SVONNode& CurrentNode = CurrentNavVolume->GetNavData().OctreeData.GetNode(NavLink);
			FVector CurrentNodePosition;

			bool bIsValid = CurrentNavVolume->GetNavData().GetLinkPosition(NavLink, CurrentNodePosition);

			DrawDebugLine(GetWorld(), GetPawnPosition(), CurrentNodePosition, bIsValid ? FColor::Green : FColor::Red, false, -1.f, 0, 10.f);
			DrawDebugString(GetWorld(), GetPawnPosition() + FVector(0.f, 0.f, -50.f), NavLink.ToString(), NULL, FColor::Yellow, 0.01f);
		}
	}
	return NavLink;
}

bool USVONNavigationComponent::FindPathAsync(const FVector& StartPosition, const FVector& TargetPosition, FThreadSafeBool& CompleteFlag, FSVONNavPathSharedPtr* OutNavPath)
{
	UE_LOG(UESVON, Log, TEXT("Finding path from %s and %s"), *StartPosition.ToString(), *TargetPosition.ToString());
	SVONLink StartNavLink;
	SVONLink TargetNavLink;
	if (HasNavData())
	{
		// Get the nav link from our volume
		if (!SVONMediator::GetLinkFromPosition(StartPosition, *CurrentNavVolume, StartNavLink))
		{
			UE_LOG(UESVON, Error, TEXT("Path finder failed to find start nav link. Is your pawn blocking the channel you've selected to generate the nav data with?"));
			return false;
		}

		if (!SVONMediator::GetLinkFromPosition(TargetPosition, *CurrentNavVolume, TargetNavLink))
		{
			UE_LOG(UESVON, Error, TEXT("Path finder failed to find target nav link"));
			return false;
		}

		SVONPathFinderSettings Settings;
		Settings.UseUnitCost = bUseUnitCost;
		Settings.UnitCost = UnitCost;
		Settings.EstimateWeight = EstimateWeight;
		Settings.NodeSizeCompensation = NodeSizeCompensation;
		Settings.PathCostType = PathCostType;
		Settings.SmoothingIterations = SmoothingIterations;

		(new FAutoDeleteAsyncTask<FSVONFindPathTask>(CurrentNavVolume->GetNavData(), Settings, GetWorld(), StartNavLink, TargetNavLink, StartPosition, TargetPosition, OutNavPath, CompleteFlag))->StartBackgroundTask();

		return true;
	}
	else
	{
		UE_LOG(UESVON, Error, TEXT("Pawn is not inside an SVON volume, or nav data has not been generated"));
	}
	return false;
}

bool USVONNavigationComponent::FindPathImmediate(const FVector& StartPosition, const FVector& TargetPosition, FSVONNavPathSharedPtr* NavPath)
{
	UE_LOG(UESVON, Log, TEXT("Finding path immediate from %s and %s"), *StartPosition.ToString(), *TargetPosition.ToString());

	SVONLink StartNavLink;
	SVONLink TargetNavLink;
	if (HasNavData())
	{
		// Get the nav link from our volume
		if (!SVONMediator::GetLinkFromPosition(StartPosition, *CurrentNavVolume, StartNavLink))
		{
			UE_LOG(UESVON, Error, TEXT("Path finder failed to find start nav link"));
			return false;
		}

		if (!SVONMediator::GetLinkFromPosition(TargetPosition, *CurrentNavVolume, TargetNavLink))
		{
			UE_LOG(UESVON, Error, TEXT("Path finder failed to find target nav link"));
			return false;
		}

		if (!NavPath || !NavPath->IsValid())
		{
			UE_LOG(UESVON, Error, TEXT("Nav path data invalid"));
			return false;
		}

		FSVONNavigationPath* Path = NavPath->Get();

		Path->ResetForRepath();

		SVONPathFinderSettings Settings;
		Settings.UseUnitCost = bUseUnitCost;
		Settings.UnitCost = UnitCost;
		Settings.EstimateWeight = EstimateWeight;
		Settings.NodeSizeCompensation = NodeSizeCompensation;
		Settings.PathCostType = PathCostType;
		Settings.SmoothingIterations = SmoothingIterations;

		SVONPathFinder pathFinder(GetWorld(), CurrentNavVolume->GetNavData(), Settings);

		int32 Result = pathFinder.FindPath(StartNavLink, TargetNavLink, StartPosition, TargetPosition, NavPath);

		Path->SetIsReady(true);

		return true;
	}
	else
	{
		UE_LOG(UESVON, Error, TEXT("Pawn is not inside an SVON volume, or nav data has not been generated"));
	}

	return false;
}

void USVONNavigationComponent::FindPathImmediate(const FVector& StartPosition, const FVector& TargetPosition, TArray<FVector>& OutPathPoints)
{
	FindPathImmediate(StartPosition, TargetPosition, &CurrentPath);

	OutPathPoints.Empty();

	for (const FSVONPathPoint& PathPoint : CurrentPath->GetPathPoints())
	{
		OutPathPoints.Emplace(PathPoint.Position);
	}
}

void USVONNavigationComponent::SetCurrentNavVolume(const ASVONVolume* Volume)
{
	CurrentNavVolume = Volume;

	if (CurrentNavVolume && CurrentNavVolume->bIsReadyForNavigation)
	{
		FVector Location = GetPawnPosition();
		if (bDebugPrintMortonCodes)
		{
			DebugLocalPosition(Location);
		}
	}
}

void USVONNavigationComponent::DebugLocalPosition(FVector& aPosition)
{
	if (HasNavData())
	{
		for (int i = 0; i < CurrentNavVolume->GetNavData().OctreeData.GetNumLayers() - 1; i++)
		{
			FIntVector Pos;
			SVONMediator::GetVolumeXYZ(GetPawnPosition(), *CurrentNavVolume, i, Pos);
			uint_fast64_t Code = morton3D_64_encode(Pos.X, Pos.Y, Pos.Z);
			FString CodeString = FString::FromInt(Code);
			DrawDebugString(GetWorld(), GetPawnPosition() + FVector(0.f, 0.f, i * 50.0f), Pos.ToString() + " - " + CodeString, NULL, FColor::White, 0.01f);
		}
	}
}

FVector USVONNavigationComponent::GetPawnPosition() const
{
	FVector Result;

	AController* Controller = Cast<AController>(GetOwner());

	if (Controller)
	{
		if (APawn* Pawn = Controller->GetPawn())
			Result = Pawn->GetActorLocation();
	}

	return Result;
}
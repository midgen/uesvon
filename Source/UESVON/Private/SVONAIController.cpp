// Fill out your copyright notice in the Description page of Project Settings.

#include "SVONAIController.h"

#include "AI/Navigation/NavigationSystem.h"
#include "SVONNavigationComponent.h"

#include "Kismet/GameplayStatics.h"
#include "DisplayDebugHelpers.h"


DECLARE_CYCLE_STAT(TEXT("SVONMoveTo"), STAT_SVONMoveTo, STATGROUP_AI);

DEFINE_LOG_CATEGORY(LogAINavigation);


ASVONAIController::ASVONAIController(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	
	SVONNavComponent = CreateDefaultSubobject<USVONNavigationComponent>(TEXT("SVONNavigationComponent"));

	myNavPath = MakeShareable<FNavigationPath>(new FNavigationPath());
}

FPathFollowingRequestResult ASVONAIController::MoveTo(const FAIMoveRequest& MoveRequest, FNavPathSharedPtr* OutPath /*= nullptr*/)
{
	// both MoveToActor and MoveToLocation can be called from blueprints/script and should keep only single movement request at the same time.
	// this function is entry point of all movement mechanics - do NOT abort in here, since movement may be handled by AITasks, which support stacking 

	SCOPE_CYCLE_COUNTER(STAT_SVONMoveTo);
	UE_VLOG(this, LogAINavigation, Log, TEXT("MoveTo: %s"), *MoveRequest.ToString());

	FPathFollowingRequestResult ResultData;
	ResultData.Code = EPathFollowingRequestResult::Failed;

	if (MoveRequest.IsValid() == false)
	{
		UE_VLOG(this, LogAINavigation, Error, TEXT("MoveTo request failed due MoveRequest not being valid. Most probably desireg Goal Actor not longer exists"), *MoveRequest.ToString());
		return ResultData;
	}

	if (GetPathFollowingComponent() == nullptr)
	{
		UE_VLOG(this, LogAINavigation, Error, TEXT("MoveTo request failed due missing PathFollowingComponent"));
		return ResultData;
	}

	ensure(MoveRequest.GetNavigationFilter() || !DefaultNavigationFilterClass);

	bool bCanRequestMove = true;
	bool bAlreadyAtGoal = false;

	if (!MoveRequest.IsMoveToActorRequest())
	{
		if (MoveRequest.GetGoalLocation().ContainsNaN() || FAISystem::IsValidLocation(MoveRequest.GetGoalLocation()) == false)
		{
			UE_VLOG(this, LogAINavigation, Error, TEXT("AAIController::MoveTo: Destination is not valid! Goal(%s)"), TEXT_AI_LOCATION(MoveRequest.GetGoalLocation()));
			bCanRequestMove = false;
		}

		bAlreadyAtGoal = bCanRequestMove && GetPathFollowingComponent()->HasReached(MoveRequest);
	}
	else
	{
		bAlreadyAtGoal = bCanRequestMove && GetPathFollowingComponent()->HasReached(MoveRequest);
	}

	if (bAlreadyAtGoal)
	{
		UE_VLOG(this, LogAINavigation, Log, TEXT("MoveTo: already at goal!"));
		ResultData.MoveId = GetPathFollowingComponent()->RequestMoveWithImmediateFinish(EPathFollowingResult::Success);
		ResultData.Code = EPathFollowingRequestResult::AlreadyAtGoal;
	}
	else if (bCanRequestMove)
	{
		SVONNavComponent->FindPathImmediate(GetPawn()->GetActorLocation(), MoveRequest.GetGoalLocation(), &myNavPath);

		const FAIRequestID RequestID = myNavPath.IsValid() ? RequestMove(MoveRequest, myNavPath) : FAIRequestID::InvalidRequest;

		if(RequestID.IsValid())
		{
	
			*OutPath = myNavPath;
			bAllowStrafe = MoveRequest.CanStrafe();
			ResultData.MoveId = RequestID;
			ResultData.Code = EPathFollowingRequestResult::RequestSuccessful;
		}
		
	}

	if (ResultData.Code == EPathFollowingRequestResult::Failed)
	{
		ResultData.MoveId = GetPathFollowingComponent()->RequestMoveWithImmediateFinish(EPathFollowingResult::Invalid);
	}

	return ResultData;
}

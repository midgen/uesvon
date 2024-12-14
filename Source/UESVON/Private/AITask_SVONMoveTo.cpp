// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include <UESVON/Public/AITask_SVONMoveTo.h>
#include <UESVON/Public/UESVON.h>
#include <UESVON/Public/SVONNavigationComponent.h>
#include <UESVON/Public/SVONNavigationPath.h>
#include <UESVON/Public/SVONVolume.h>

#include <Runtime/AIModule/Classes/AIController.h>
#include <Runtime/AIModule/Classes/AIResources.h>
#include <Runtime/AIModule/Classes/AISystem.h>
#include <Runtime/Engine/Public/VisualLogger/VisualLogger.h>
#include <Runtime/Engine/Public/TimerManager.h>
#include <Runtime/GameplayTasks/Classes/GameplayTasksComponent.h>


UAITask_SVONMoveTo::UAITask_SVONMoveTo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsPausable = true;
	MoveRequestID = FAIRequestID::InvalidRequest;

	MoveRequest.SetAcceptanceRadius(GET_AI_CONFIG_VAR(AcceptanceRadius));
	MoveRequest.SetReachTestIncludesAgentRadius(GET_AI_CONFIG_VAR(bFinishMoveOnGoalOverlap));
	MoveRequest.SetAllowPartialPath(GET_AI_CONFIG_VAR(bAcceptPartialPaths));
	MoveRequest.SetUsePathfinding(true);

	Result.Code = ESVONPathfindingRequestResult::Failed;

	AddRequiredResource(UAIResource_Movement::StaticClass());
	AddClaimedResource(UAIResource_Movement::StaticClass());

	MoveResult = EPathFollowingResult::Invalid;

	Path = MakeShareable<FNavigationPath>(new FNavigationPath());
}

UAITask_SVONMoveTo* UAITask_SVONMoveTo::SVONAIMoveTo(AAIController* Controller, FVector InGoalLocation, bool bUseAsyncPathfinding, AActor* InGoalActor,
	float AcceptanceRadius, EAIOptionFlag::Type StopOnOverlap, bool bLockAILogic, bool bUseContinuosGoalTracking)
{
	UAITask_SVONMoveTo* MyTask = Controller ? UAITask::NewAITask<UAITask_SVONMoveTo>(*Controller, EAITaskPriority::High) : nullptr;
	if (MyTask)
	{
		MyTask->bUseAsyncPathfinding = bUseAsyncPathfinding;
		// We need to tick the task if we're using async, to check when results are back
		MyTask->bTickingTask = bUseAsyncPathfinding;

		FAIMoveRequest MoveReq;
		if (InGoalActor)
		{
			MoveReq.SetGoalActor(InGoalActor);
		}
		else
		{
			MoveReq.SetGoalLocation(InGoalLocation);
		}

		MoveReq.SetAcceptanceRadius(AcceptanceRadius);
		MoveReq.SetReachTestIncludesAgentRadius(FAISystem::PickAIOption(StopOnOverlap, MoveReq.IsReachTestIncludingAgentRadius()));
		if (Controller)
		{
			MoveReq.SetNavigationFilter(Controller->GetDefaultNavigationFilterClass());
		}

		MyTask->SetUp(Controller, MoveReq, bUseAsyncPathfinding);
		MyTask->SetContinuousGoalTracking(bUseContinuosGoalTracking);

		if (bLockAILogic)
		{
			MyTask->RequestAILogicLocking();
		}
	}

	return MyTask;
}

void UAITask_SVONMoveTo::SetUp(AAIController* Controller, const FAIMoveRequest& InMoveRequest, bool UseAsyncPathfinding)
{
	OwnerController = Controller;
	MoveRequest = InMoveRequest;
	bUseAsyncPathfinding = UseAsyncPathfinding;
	bTickingTask = bUseAsyncPathfinding;

	// Fail if no nav component
	NavComponent = Cast<USVONNavigationComponent>(GetOwnerActor()->GetComponentByClass(USVONNavigationComponent::StaticClass()));
	if (!NavComponent)
	{
		UE_VLOG(this, VUESVON, Error, TEXT("SVONMoveTo request failed due missing SVONNavComponent"), *MoveRequest.ToString());
		UE_LOG(UESVON, Error, TEXT("SVONMoveTo request failed due missing SVONNavComponent on the pawn"));
		return;
	}

	// Use the path instance from the navcomponent
	SVONPath = NavComponent->GetPath();
}

void UAITask_SVONMoveTo::SetContinuousGoalTracking(bool bEnable)
{
	bUseContinuousTracking = bEnable;
}

void UAITask_SVONMoveTo::TickTask(float DeltaTime)
{
	if (AsyncTaskComplete)
		HandleAsyncPathTaskComplete();
}

void UAITask_SVONMoveTo::FinishMoveTask(EPathFollowingResult::Type InResult)
{
	if (MoveRequestID.IsValid())
	{
		UPathFollowingComponent* PFComp = OwnerController ? OwnerController->GetPathFollowingComponent() : nullptr;
		if (PFComp && PFComp->GetStatus() != EPathFollowingStatus::Idle)
		{
			ResetObservers();
			PFComp->AbortMove(*this, FPathFollowingResultFlags::OwnerFinished, MoveRequestID);
		}
	}

	MoveResult = InResult;
	EndTask();

	if (InResult == EPathFollowingResult::Invalid)
	{
		OnRequestFailed.Broadcast();
	}
	else
	{
		OnMoveFinished.Broadcast(InResult, OwnerController);
	}
}

void UAITask_SVONMoveTo::Activate()
{
	Super::Activate();

	UE_CVLOG(bUseContinuousTracking, GetGameplayTasksComponent(), LogGameplayTasks, Log, TEXT("Continuous goal tracking requested, moving to: %s"),
		MoveRequest.IsMoveToActorRequest() ? TEXT("actor => looping successful moves!") : TEXT("location => will NOT loop"));

	MoveRequestID = FAIRequestID::InvalidRequest;
	ConditionalPerformMove();
}

void UAITask_SVONMoveTo::ConditionalPerformMove()
{
	if (MoveRequest.IsUsingPathfinding() && OwnerController && OwnerController->ShouldPostponePathUpdates())
	{
		UE_VLOG(GetGameplayTasksComponent(), LogGameplayTasks, Log, TEXT("%s> can't path right now, waiting..."), *GetName());
		OwnerController->GetWorldTimerManager().SetTimer(MoveRetryTimerHandle, this, &UAITask_SVONMoveTo::ConditionalPerformMove, 0.2f, false);
	}
	else
	{
		MoveRetryTimerHandle.Invalidate();
		PerformMove();
	}
}

void UAITask_SVONMoveTo::PerformMove()
{
	// Prepare the move first (check for early out)
	CheckPathPreConditions();

	ResetObservers();
	ResetTimers();
	ResetPaths();




	if (Result.Code == ESVONPathfindingRequestResult::AlreadyAtGoal)
	{
		MoveRequestID = Result.MoveId;
		OnRequestFinished(Result.MoveId, FPathFollowingResult(EPathFollowingResult::Success, FPathFollowingResultFlags::AlreadyAtGoal));
		return;
	}
	
	// If we're ready to path, then request the path
	if (Result.Code == ESVONPathfindingRequestResult::ReadyToPath)
	{
		bUseAsyncPathfinding ? RequestPathAsync() : RequestPathSynchronous();
	
		switch (Result.Code)
		{
		case ESVONPathfindingRequestResult::Failed:
			FinishMoveTask(EPathFollowingResult::Invalid);
			break;
		case ESVONPathfindingRequestResult::Success: // Synchronous pathfinding
			MoveRequestID = Result.MoveId;
			if (IsFinished())
			{
				UE_VLOG(GetGameplayTasksComponent(), LogGameplayTasks, Error, TEXT("%s> re-Activating Finished task!"), *GetName());
			}
			RequestMove(); // Start the move
			break;
		case ESVONPathfindingRequestResult::Deferred: // Async...we're waiting on the task to return
			MoveRequestID = Result.MoveId;
			AsyncTaskComplete = false;
			break;
		default:
			checkNoEntry();
			break;
		}

	}
}

void UAITask_SVONMoveTo::Pause()
{
	if (OwnerController && MoveRequestID.IsValid())
	{
		OwnerController->PauseMove(MoveRequestID);
	}

	ResetTimers();
	Super::Pause();
}

void UAITask_SVONMoveTo::Resume()
{
	Super::Resume();

	if (!MoveRequestID.IsValid() || (OwnerController && !OwnerController->ResumeMove(MoveRequestID)))
	{
		UE_CVLOG(MoveRequestID.IsValid(), GetGameplayTasksComponent(), LogGameplayTasks, Log, TEXT("%s> Resume move failed, starting new one."), *GetName());
		ConditionalPerformMove();
	}
}

void UAITask_SVONMoveTo::SetObservedPath(FNavPathSharedPtr InPath)
{
	if (PathUpdateDelegateHandle.IsValid() && Path.IsValid())
	{
		Path->RemoveObserver(PathUpdateDelegateHandle);
	}

	PathUpdateDelegateHandle.Reset();

	Path = InPath;
	if (Path.IsValid())
	{
		// disable auto repaths, it will be handled by move task to include ShouldPostponePathUpdates condition
		Path->EnableRecalculationOnInvalidation(false);
		PathUpdateDelegateHandle = Path->AddObserver(FNavigationPath::FPathObserverDelegate::FDelegate::CreateUObject(this, &UAITask_SVONMoveTo::OnPathEvent));
	}
}



void UAITask_SVONMoveTo::CheckPathPreConditions()
{
	UE_VLOG(this, VUESVON, Log, TEXT("SVONMoveTo: %s"), *MoveRequest.ToString());

	Result.Code = ESVONPathfindingRequestResult::Failed;

	// Fail if no nav component
	NavComponent = Cast<USVONNavigationComponent>(GetOwnerActor()->GetComponentByClass(USVONNavigationComponent::StaticClass()));
	if (!NavComponent)
	{
		UE_VLOG(this, VUESVON, Error, TEXT("SVONMoveTo request failed due missing SVONNavComponent on the pawn"), *MoveRequest.ToString());
		UE_LOG(UESVON, Error, TEXT("SVONMoveTo request failed due missing SVONNavComponent on the pawn"));
		return;
	}

	if (MoveRequest.IsValid() == false)
	{
		UE_VLOG(this, VUESVON, Error, TEXT("SVONMoveTo request failed due MoveRequest not being valid. Most probably desired Goal Actor not longer exists"), *MoveRequest.ToString());
		UE_LOG(UESVON, Error, TEXT("SVONMoveTo request failed due MoveRequest not being valid. Most probably desired Goal Actor not longer exists"));
		return;
	}

	if (OwnerController->GetPathFollowingComponent() == nullptr)
	{
		UE_VLOG(this, VUESVON, Error, TEXT("SVONMoveTo request failed due missing PathFollowingComponent"));
		UE_LOG(UESVON, Error, TEXT("SVONMoveTo request failed due missing PathFollowingComponent"));
		return;
	}

	bool bCanRequestMove = true;
	bool bAlreadyAtGoal = false;

	if (!MoveRequest.IsMoveToActorRequest())
	{
		if (MoveRequest.GetGoalLocation().ContainsNaN() || FAISystem::IsValidLocation(MoveRequest.GetGoalLocation()) == false)
		{
			UE_VLOG(this, VUESVON, Error, TEXT("SVONMoveTo: Destination is not valid! Goal(%s)"), TEXT_AI_LOCATION(MoveRequest.GetGoalLocation()));
			UE_LOG(UESVON, Error, TEXT("SVONMoveTo: Destination is not valid! Goal(%s)"), TEXT_AI_LOCATION(MoveRequest.GetGoalLocation()));
			bCanRequestMove = false;
		}

		bAlreadyAtGoal = bCanRequestMove && OwnerController->GetPathFollowingComponent()->HasReached(MoveRequest);
	}
	else
	{
		bAlreadyAtGoal = bCanRequestMove && OwnerController->GetPathFollowingComponent()->HasReached(MoveRequest);
	}

	if (bAlreadyAtGoal)
	{
		UE_VLOG(this, VUESVON, Log, TEXT("SVONMoveTo: already at goal!"));
		UE_LOG(UESVON, Log, TEXT("SVONMoveTo: already at goal!"));
		Result.MoveId = OwnerController->GetPathFollowingComponent()->RequestMoveWithImmediateFinish(EPathFollowingResult::Success);
		Result.Code = ESVONPathfindingRequestResult::AlreadyAtGoal;
	}

	if (bCanRequestMove)
	{
		Result.Code = ESVONPathfindingRequestResult::ReadyToPath;
	}

	return;
	
}

void UAITask_SVONMoveTo::RequestPathSynchronous()
{
	Result.Code = ESVONPathfindingRequestResult::Failed;

	UE_VLOG(this, VUESVON, Log, TEXT("SVONMoveTo: Requesting Synchronous pathfinding!"));
	UE_LOG(UESVON, Error, TEXT("SVONMoveTo: Requesting Synchronous pathfinding!"));

	if (NavComponent->FindPathImmediate(NavComponent->GetPawnPosition(), MoveRequest.IsMoveToActorRequest() ? MoveRequest.GetGoalActor()->GetActorLocation() : MoveRequest.GetGoalLocation(), &SVONPath))
	{
		Result.Code = ESVONPathfindingRequestResult::Success;
	}


	return;
}

void UAITask_SVONMoveTo::RequestPathAsync()
{
	Result.Code = ESVONPathfindingRequestResult::Failed;

	// Fail if no nav component
	USVONNavigationComponent* svonNavComponent = Cast<USVONNavigationComponent>(GetOwnerActor()->GetComponentByClass(USVONNavigationComponent::StaticClass()));
	if (!svonNavComponent)
		return;

	AsyncTaskComplete = false;

	// Request the async path
	svonNavComponent->FindPathAsync(NavComponent->GetPawnPosition(), MoveRequest.IsMoveToActorRequest() ? MoveRequest.GetGoalActor()->GetActorLocation() : MoveRequest.GetGoalLocation(), AsyncTaskComplete, &SVONPath);

	Result.Code = ESVONPathfindingRequestResult::Deferred;
}

/* Requests the move, based on the current path */
void UAITask_SVONMoveTo::RequestMove()
{
	Result.Code = ESVONPathfindingRequestResult::Failed;

	LogPathHelper();

	// Copy the SVO path into a regular path for now, until we implement our own path follower.
	SVONPath->CreateNavPath(*Path);
	Path->MarkReady();

	UPathFollowingComponent* PFComp = OwnerController ? OwnerController->GetPathFollowingComponent() : nullptr;
	if (PFComp == nullptr)
	{
		FinishMoveTask(EPathFollowingResult::Invalid);
		return;
	}

	PathFinishDelegateHandle = PFComp->OnRequestFinished.AddUObject(this, &UAITask_SVONMoveTo::OnRequestFinished);
	SetObservedPath(Path);

	const FAIRequestID RequestId = Path->IsValid() ? OwnerController->RequestMove(MoveRequest, Path) : FAIRequestID::InvalidRequest;

	if (RequestId.IsValid())
	{
		UE_VLOG(this, VUESVON, Log, TEXT("SVON Pathfinding successful, moving"));
		UE_LOG(UESVON, Log, TEXT("SVON Pathfinding successful, moving"));
		Result.MoveId = RequestId;
		Result.Code = ESVONPathfindingRequestResult::Success;
	}

	if (Result.Code == ESVONPathfindingRequestResult::Failed)
	{
		Result.MoveId = OwnerController->GetPathFollowingComponent()->RequestMoveWithImmediateFinish(EPathFollowingResult::Invalid);
	}
}

void UAITask_SVONMoveTo::HandleAsyncPathTaskComplete()
{
	Result.Code = ESVONPathfindingRequestResult::Success;
	// Request the move
	RequestMove();
	// Flag that we've processed the task
	AsyncTaskComplete = false;

}

void UAITask_SVONMoveTo::ResetPaths()
{
	if(Path.IsValid())
		Path->ResetForRepath();

	if(SVONPath.IsValid())
		SVONPath->ResetForRepath();
}

/** Renders the octree path as a 3d tunnel in the visual logger */
void UAITask_SVONMoveTo::LogPathHelper()
{
#if WITH_EDITOR
#if ENABLE_VISUAL_LOG

	USVONNavigationComponent* svonNavComponent = Cast<USVONNavigationComponent>(GetOwnerActor()->GetComponentByClass(USVONNavigationComponent::StaticClass()));
	if (!svonNavComponent)
		return;

	FVisualLogger& Vlog = FVisualLogger::Get();
	if (Vlog.IsRecording() &&
		SVONPath.IsValid() && SVONPath.Get()->GetPathPoints().Num())
	{

		FVisualLogEntry* Entry = FVisualLogger::GetEntryToWrite(OwnerController->GetPawn(), VUESVON);
		if (Entry)
		{
			for (int i = 0; i < SVONPath->GetPathPoints().Num(); i++)
			{
				if (i == 0 || i == SVONPath->GetPathPoints().Num() - 1)
					continue;

				const FSVONPathPoint& Point = SVONPath->GetPathPoints()[i];
	
				float size = 0.f;

				if (Point.myLayer == 0)
				{
					size = svonNavComponent->GetCurrentVolume()->GetNavData().GetVoxelSize(0) * 0.25f;
				}
				else 
				{
					size = svonNavComponent->GetCurrentVolume()->GetNavData().GetVoxelSize(Point.myLayer - 1);
				}


				UE_VLOG_BOX(OwnerController->GetPawn(), VUESVON, Verbose, FBox(Point.myPosition + FVector(size * 0.5f), Point.myPosition - FVector(size * 0.5f)), FColor::Black, TEXT_EMPTY);

			}
		}

	}
#endif // ENABLE_VISUAL_LOG
#endif // WITH_EDITOR
}

void UAITask_SVONMoveTo::ResetObservers()
{
	if (Path.IsValid())
	{
		Path->DisableGoalActorObservation();
	}

	if (PathFinishDelegateHandle.IsValid())
	{
		UPathFollowingComponent* PFComp = OwnerController ? OwnerController->GetPathFollowingComponent() : nullptr;
		if (PFComp)
		{
			PFComp->OnRequestFinished.Remove(PathFinishDelegateHandle);
		}

		PathFinishDelegateHandle.Reset();
	}

	if (PathUpdateDelegateHandle.IsValid())
	{
		if (Path.IsValid())
		{
			Path->RemoveObserver(PathUpdateDelegateHandle);
		}

		PathUpdateDelegateHandle.Reset();
	}
}

void UAITask_SVONMoveTo::ResetTimers()
{
	if (MoveRetryTimerHandle.IsValid())
	{
		if (OwnerController)
		{
			OwnerController->GetWorldTimerManager().ClearTimer(MoveRetryTimerHandle);
		}

		MoveRetryTimerHandle.Invalidate();
	}

	if (PathRetryTimerHandle.IsValid())
	{
		if (OwnerController)
		{
			OwnerController->GetWorldTimerManager().ClearTimer(PathRetryTimerHandle);
		}

		PathRetryTimerHandle.Invalidate();
	}
}

void UAITask_SVONMoveTo::OnDestroy(bool bInOwnerFinished)
{
	Super::OnDestroy(bInOwnerFinished);

	ResetObservers();
	ResetTimers();

	if (MoveRequestID.IsValid())
	{
		UPathFollowingComponent* PfComp = OwnerController ? OwnerController->GetPathFollowingComponent() : nullptr;
		if (PfComp && PfComp->GetStatus() != EPathFollowingStatus::Idle)
		{
			PfComp->AbortMove(*this, FPathFollowingResultFlags::OwnerFinished, MoveRequestID);
		}
	}

	// clear the shared pointer now to make sure other systems
	// don't think this path is still being used
	Path = nullptr;
	SVONPath = nullptr;
}

void UAITask_SVONMoveTo::OnRequestFinished(FAIRequestID RequestId, const FPathFollowingResult& InResult)
{
	FinishMoveTask(InResult.Code);
}

void UAITask_SVONMoveTo::OnPathEvent(FNavigationPath* InPath, ENavPathEvent::Type Event)
{
	const static UEnum* NavPathEventEnum = FindObject<UEnum>(this, TEXT("ENavPathEvent"));
	UE_VLOG(GetGameplayTasksComponent(), LogGameplayTasks, Log, TEXT("%s> Path event: %s"), *GetName(), *NavPathEventEnum->GetNameStringByValue(Event));

	switch (Event)
	{
	case ENavPathEvent::NewPath:
	case ENavPathEvent::UpdatedDueToGoalMoved:
	case ENavPathEvent::UpdatedDueToNavigationChanged:
		if (InPath && InPath->IsPartial() && !MoveRequest.IsUsingPartialPaths())
		{
			UE_VLOG(GetGameplayTasksComponent(), LogGameplayTasks, Log, TEXT(">> partial path is not allowed, aborting"));
			UPathFollowingComponent::LogPathHelper(OwnerController, InPath, MoveRequest.GetGoalActor());
			FinishMoveTask(EPathFollowingResult::Aborted);
		}
#if ENABLE_VISUAL_LOG
		else if (!IsActive())
		{
			UPathFollowingComponent::LogPathHelper(OwnerController, InPath, MoveRequest.GetGoalActor());
		}
#endif // ENABLE_VISUAL_LOG
		break;

	case ENavPathEvent::Invalidated:
		ConditionalUpdatePath();
		break;

	case ENavPathEvent::Cleared:
	case ENavPathEvent::RePathFailed:
		UE_VLOG(GetGameplayTasksComponent(), LogGameplayTasks, Log, TEXT(">> no path, aborting!"));
		FinishMoveTask(EPathFollowingResult::Aborted);
		break;

	case ENavPathEvent::MetaPathUpdate:
	default:
		break;
	}
}

void UAITask_SVONMoveTo::ConditionalUpdatePath()
{
	// mark this path as waiting for repath so that PathFollowingComponent doesn't abort the move while we 
	// micro manage repathing moment
	// note that this flag fill get cleared upon repathing end
	if (Path.IsValid())
	{
		Path->SetManualRepathWaiting(true);
	}

	if (MoveRequest.IsUsingPathfinding() && OwnerController && OwnerController->ShouldPostponePathUpdates())
	{
		UE_VLOG(GetGameplayTasksComponent(), LogGameplayTasks, Log, TEXT("%s> can't path right now, waiting..."), *GetName());
		OwnerController->GetWorldTimerManager().SetTimer(PathRetryTimerHandle, this, &UAITask_SVONMoveTo::ConditionalUpdatePath, 0.2f, false);
	}
	else
	{
		PathRetryTimerHandle.Invalidate();

		ANavigationData* NavData = Path.IsValid() ? Path->GetNavigationDataUsed() : nullptr;
		if (NavData)
		{
			NavData->RequestRePath(Path, ENavPathUpdateType::NavigationChanged);
		}
		else
		{
			UE_VLOG(GetGameplayTasksComponent(), LogGameplayTasks, Log, TEXT("%s> unable to repath, aborting!"), *GetName());
			FinishMoveTask(EPathFollowingResult::Aborted);
		}
	}
}

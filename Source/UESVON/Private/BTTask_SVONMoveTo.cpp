
#include "UESVON/Public/BTTask_SVONMoveTo.h"
#include "UESVON/Public/AITask_SVONMoveTo.h"

#include <Runtime/AIModule/Classes/AIController.h>
#include <Runtime/AIModule/Classes/AISystem.h>
#include <Runtime/AIModule/Classes/BehaviorTree/Blackboard/BlackboardKeyType_Object.h>
#include <Runtime/AIModule/Classes/BehaviorTree/Blackboard/BlackboardKeyType_Vector.h>
#include <Runtime/AIModule/Classes/BehaviorTree/BlackboardComponent.h>
#include <Runtime/AIModule/Classes/Navigation/PathFollowingComponent.h>

#include <Runtime/Engine/Classes/GameFramework/Actor.h>
#include <Runtime/Engine/Public/VisualLogger/VisualLogger.h>

UBTTask_SVONMoveTo::UBTTask_SVONMoveTo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "SVON Move To";
	bNotifyTick = false;
	bNotifyTaskFinished = true;

	AcceptableRadius = GET_AI_CONFIG_VAR(AcceptanceRadius);
	bReachTestIncludesGoalRadius = bReachTestIncludesAgentRadius = GET_AI_CONFIG_VAR(bFinishMoveOnGoalOverlap);
	bTrackMovingGoal = true;
	ObservedBlackboardValueTolerance = AcceptableRadius * 0.95f;
	bUseAsyncPathfinding = false;

	// accept only actors and vectors
	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_SVONMoveTo, BlackboardKey), AActor::StaticClass());
	BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_SVONMoveTo, BlackboardKey));
}

EBTNodeResult::Type UBTTask_SVONMoveTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type NodeResult = EBTNodeResult::InProgress;

	FBTSVONMoveToTaskMemory* MyMemory = reinterpret_cast<FBTSVONMoveToTaskMemory*>(NodeMemory);
	MyMemory->PreviousGoalLocation = FAISystem::InvalidLocation;
	MyMemory->MoveRequestID = FAIRequestID::InvalidRequest;

	AAIController* MyController = OwnerComp.GetAIOwner();
	MyMemory->bWaitingForPath = false;
	if (!MyMemory->bWaitingForPath)
	{
		NodeResult = PerformMoveTask(OwnerComp, NodeMemory);
	}
	else
	{
		UE_VLOG(MyController, LogBehaviorTree, Log, TEXT("Pathfinding requests are freezed, waiting..."));
	}

	if (NodeResult == EBTNodeResult::InProgress && bObserveBlackboardValue)
	{
		UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
		if (ensure(BlackboardComp))
		{
			if (MyMemory->BBObserverDelegateHandle.IsValid())
			{
				UE_VLOG(MyController, LogBehaviorTree, Warning, TEXT("UBTTask_SVONMoveTo::ExecuteTask \'%s\' Old BBObserverDelegateHandle is still valid! Removing old Observer."), *GetNodeName());
				BlackboardComp->UnregisterObserver(BlackboardKey.GetSelectedKeyID(), MyMemory->BBObserverDelegateHandle);
			}
			MyMemory->BBObserverDelegateHandle = BlackboardComp->RegisterObserver(BlackboardKey.GetSelectedKeyID(), this, FOnBlackboardChangeNotification::CreateUObject(this, &UBTTask_SVONMoveTo::OnBlackboardValueChange));
		}
	}

	return NodeResult;
}

EBTNodeResult::Type UBTTask_SVONMoveTo::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTSVONMoveToTaskMemory* MyMemory = reinterpret_cast<FBTSVONMoveToTaskMemory*>(NodeMemory);
	if (!MyMemory->bWaitingForPath)
	{
		if (MyMemory->MoveRequestID.IsValid())
		{
			AAIController* MyController = OwnerComp.GetAIOwner();
			if (MyController && MyController->GetPathFollowingComponent())
			{
				MyController->GetPathFollowingComponent()->AbortMove(*this, FPathFollowingResultFlags::OwnerFinished, MyMemory->MoveRequestID);
			}
		}
		else
		{
			MyMemory->bObserverCanFinishTask = false;
			UAITask_SVONMoveTo* MoveTask = MyMemory->Task.Get();
			if (MoveTask)
			{
				MoveTask->ExternalCancel();
			}
			else
			{
				UE_VLOG(&OwnerComp, LogBehaviorTree, Error, TEXT("Can't abort path following! bWaitingForPath:false, MoveRequestID:invalid, MoveTask:none!"));
			}
		}
	}

	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_SVONMoveTo::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	FBTSVONMoveToTaskMemory* MyMemory = reinterpret_cast<FBTSVONMoveToTaskMemory*>(NodeMemory);
	MyMemory->Task.Reset();

	if (bObserveBlackboardValue)
	{
		UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
		if (ensure(BlackboardComp) && MyMemory->BBObserverDelegateHandle.IsValid())
		{
			BlackboardComp->UnregisterObserver(BlackboardKey.GetSelectedKeyID(), MyMemory->BBObserverDelegateHandle);
		}

		MyMemory->BBObserverDelegateHandle.Reset();
	}

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBTTask_SVONMoveTo::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTSVONMoveToTaskMemory* MyMemory = (FBTSVONMoveToTaskMemory*)NodeMemory;
	if (MyMemory->bWaitingForPath && !OwnerComp.IsPaused())
	{
		AAIController* MyController = OwnerComp.GetAIOwner();
		if (MyController && !MyController->ShouldPostponePathUpdates())
		{
			UE_VLOG(MyController, LogBehaviorTree, Log, TEXT("Pathfinding requests are unlocked!"));
			MyMemory->bWaitingForPath = false;

			const EBTNodeResult::Type NodeResult = PerformMoveTask(OwnerComp, NodeMemory);
			if (NodeResult != EBTNodeResult::InProgress)
			{
				FinishLatentTask(OwnerComp, NodeResult);
			}
		}
	}
}

uint16 UBTTask_SVONMoveTo::GetInstanceMemorySize() const
{
	return sizeof(FBTSVONMoveToTaskMemory);
}

void UBTTask_SVONMoveTo::PostLoad()
{
	Super::PostLoad();
}

void UBTTask_SVONMoveTo::OnGameplayTaskDeactivated(UGameplayTask& Task)
{
	// AI move task finished
	UAITask_SVONMoveTo* MoveTask = Cast<UAITask_SVONMoveTo>(&Task);
	if (MoveTask && MoveTask->GetAIController() && MoveTask->GetState() != EGameplayTaskState::Paused)
	{
		UBehaviorTreeComponent* BehaviorComp = GetBTComponentForTask(Task);
		if (BehaviorComp)
		{
			uint8* RawMemory = BehaviorComp->GetNodeMemory(this, BehaviorComp->FindInstanceContainingNode(this));
			FBTSVONMoveToTaskMemory* MyMemory = reinterpret_cast<FBTSVONMoveToTaskMemory*>(RawMemory);

			if (MyMemory->bObserverCanFinishTask && (MoveTask == MyMemory->Task))
			{
				const bool bSuccess = MoveTask->WasMoveSuccessful();
				FinishLatentTask(*BehaviorComp, bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
			}
		}
	}
}

void UBTTask_SVONMoveTo::OnMessage(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, FName Message, int32 RequestID, bool bSuccess)
{
	// AIMessage_RepathFailed means task has failed
	bSuccess &= (Message != UBrainComponent::AIMessage_RepathFailed);
	Super::OnMessage(OwnerComp, NodeMemory, Message, RequestID, bSuccess);
}

EBlackboardNotificationResult UBTTask_SVONMoveTo::OnBlackboardValueChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID)
{
	UBehaviorTreeComponent* BehaviorComp = Cast<UBehaviorTreeComponent>(Blackboard.GetBrainComponent());
	if (BehaviorComp == nullptr)
	{
		return EBlackboardNotificationResult::RemoveObserver;
	}

	uint8* RawMemory = BehaviorComp->GetNodeMemory(this, BehaviorComp->FindInstanceContainingNode(this));
	FBTSVONMoveToTaskMemory* MyMemory = reinterpret_cast<FBTSVONMoveToTaskMemory*>(RawMemory);

	const EBTTaskStatus::Type TaskStatus = BehaviorComp->GetTaskStatus(this);
	if (TaskStatus != EBTTaskStatus::Active)
	{
		UE_VLOG(BehaviorComp, LogBehaviorTree, Error, TEXT("BT MoveTo \'%s\' task observing BB entry while no longer being active!"), *GetNodeName());

		// resetting BBObserverDelegateHandle without unregistering observer since
		// returning EBlackboardNotificationResult::RemoveObserver here will take care of that for us
		MyMemory->BBObserverDelegateHandle.Reset(); //-V595

		return EBlackboardNotificationResult::RemoveObserver;
	}

	// this means the move has already started. MyMemory->bWaitingForPath == true would mean we're waiting for right moment to start it anyway,
	// so we don't need to do anything due to BB value change
	if (MyMemory != nullptr && MyMemory->bWaitingForPath == false && BehaviorComp->GetAIOwner() != nullptr)
	{
		check(BehaviorComp->GetAIOwner()->GetPathFollowingComponent());

		bool bUpdateMove = true;
		// check if new goal is almost identical to previous one
		if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
		{
			const FVector TargetLocation = Blackboard.GetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID());

			bUpdateMove = (FVector::DistSquared(TargetLocation, MyMemory->PreviousGoalLocation) > FMath::Square(ObservedBlackboardValueTolerance));
		}

		if (bUpdateMove)
		{
			// don't abort move if using AI tasks - it will mess things up
			if (MyMemory->MoveRequestID.IsValid())
			{
				UE_VLOG(BehaviorComp, LogBehaviorTree, Log, TEXT("Blackboard value for goal has changed, aborting current move request"));
				StopWaitingForMessages(*BehaviorComp);
				BehaviorComp->GetAIOwner()->GetPathFollowingComponent()->AbortMove(*this, FPathFollowingResultFlags::NewRequest, MyMemory->MoveRequestID, EPathFollowingVelocityMode::Keep);
			}

			const EBTNodeResult::Type NodeResult = PerformMoveTask(*BehaviorComp, RawMemory);
			if (NodeResult != EBTNodeResult::InProgress)
			{
				FinishLatentTask(*BehaviorComp, NodeResult);
			}
		}
	}

	return EBlackboardNotificationResult::ContinueObserving;
}

void UBTTask_SVONMoveTo::DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const
{
}

FString UBTTask_SVONMoveTo::GetStaticDescription() const
{
	FString KeyDesc("invalid");
	if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass() ||
		BlackboardKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		KeyDesc = BlackboardKey.SelectedKeyName.ToString();
	}

	return FString::Printf(TEXT("%s: %s"), *Super::GetStaticDescription(), *KeyDesc);
}
#if WITH_EDITOR
FName UBTTask_SVONMoveTo::GetNodeIconName() const
{
	return FName("BTEditor.Graph.BTNode.Task.MoveTo.Icon");
}

void UBTTask_SVONMoveTo::OnNodeCreated()
{
}
#endif

EBTNodeResult::Type UBTTask_SVONMoveTo::PerformMoveTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();
	FBTSVONMoveToTaskMemory* MyMemory = reinterpret_cast<FBTSVONMoveToTaskMemory*>(NodeMemory);
	AAIController* MyController = OwnerComp.GetAIOwner();

	EBTNodeResult::Type NodeResult = EBTNodeResult::Failed;
	if (MyController && MyBlackboard)
	{
		FAIMoveRequest MoveReq;
		MoveReq.SetNavigationFilter(MyController->GetDefaultNavigationFilterClass());
		MoveReq.SetAcceptanceRadius(AcceptableRadius);
		MoveReq.SetReachTestIncludesAgentRadius(bReachTestIncludesAgentRadius);
		MoveReq.SetReachTestIncludesGoalRadius(bReachTestIncludesGoalRadius);
		MoveReq.SetUsePathfinding(true);

		if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
		{
			UObject* KeyValue = MyBlackboard->GetValue<UBlackboardKeyType_Object>(BlackboardKey.GetSelectedKeyID());
			AActor* TargetActor = Cast<AActor>(KeyValue);
			if (TargetActor)
			{
				if (bTrackMovingGoal)
				{
					MoveReq.SetGoalActor(TargetActor);
				}
				else
				{
					MoveReq.SetGoalLocation(TargetActor->GetActorLocation());
				}
			}
			else
			{
				UE_VLOG(MyController, LogBehaviorTree, Warning, TEXT("UBTTask_MoveTo::ExecuteTask tried to go to actor while BB %s entry was empty"), *BlackboardKey.SelectedKeyName.ToString());
			}
		}
		else if (BlackboardKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
		{
			const FVector TargetLocation = MyBlackboard->GetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID());
			MoveReq.SetGoalLocation(TargetLocation);

			MyMemory->PreviousGoalLocation = TargetLocation;
		}

		if (MoveReq.IsValid())
		{
			UAITask_SVONMoveTo* MoveTask = MyMemory->Task.Get();
			const bool bReuseExistingTask = (MoveTask != nullptr);

			MoveTask = PrepareMoveTask(OwnerComp, MoveTask, MoveReq);
			if (MoveTask)
			{
				MyMemory->bObserverCanFinishTask = false;

				if (bReuseExistingTask)
				{
					if (MoveTask->IsActive())
					{
						UE_VLOG(MyController, LogBehaviorTree, Verbose, TEXT("\'%s\' reusing AITask %s"), *GetNodeName(), *MoveTask->GetName());
						MoveTask->ConditionalPerformMove();
					}
					else
					{
						UE_VLOG(MyController, LogBehaviorTree, Verbose, TEXT("\'%s\' reusing AITask %s, but task is not active - handing over move performing to task mechanics"), *GetNodeName(), *MoveTask->GetName());
					}
				}
				else
				{
					MyMemory->Task = MoveTask;
					UE_VLOG(MyController, LogBehaviorTree, Verbose, TEXT("\'%s\' task implementing move with task %s"), *GetNodeName(), *MoveTask->GetName());
					MoveTask->ReadyForActivation();
				}

				MyMemory->bObserverCanFinishTask = true;
				NodeResult = (MoveTask->GetState() != EGameplayTaskState::Finished) ? EBTNodeResult::InProgress : MoveTask->WasMoveSuccessful() ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
			}
		}
	}

	return NodeResult;
}

UAITask_SVONMoveTo* UBTTask_SVONMoveTo::PrepareMoveTask(UBehaviorTreeComponent& OwnerComp, UAITask_SVONMoveTo* ExistingTask, FAIMoveRequest& MoveRequest)
{
	UAITask_SVONMoveTo* MoveTask = ExistingTask ? ExistingTask : NewBTAITask<UAITask_SVONMoveTo>(OwnerComp);
	if (MoveTask)
	{
		MoveTask->SetUp(MoveTask->GetAIController(), MoveRequest, bUseAsyncPathfinding);
	}

	return MoveTask;
}

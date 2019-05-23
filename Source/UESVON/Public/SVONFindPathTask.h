#pragma once


#include "Runtime/Core/Public/Async/AsyncWork.h"
#include "SVONLink.h"
#include "SVONTypes.h"
#include "SVONPathFinder.h"
#include "ThreadSafeBool.h"

class ASVONVolume;
struct SVONPathFinderSettings;

class FSVONFindPathTask : public FNonAbandonableTask
{
	friend class FAutoDeleteAsyncTask<FSVONFindPathTask>;

public:
	FSVONFindPathTask(ASVONVolume& aVolume, SVONPathFinderSettings& aSettings,  UWorld* aWorld, const SVONLink aStart, const SVONLink aTarget, const FVector& aStartPos, const FVector& aTargetPos, FSVONNavPathSharedPtr* oPath, FThreadSafeBool& aCompleteFlag, TArray<FVector>& aDebugOpenPoints) :
		myVolume(aVolume),
		myWorld(aWorld),
		myStart(aStart),
		myTarget(aTarget),
		myStartPos(aStartPos),
		myTargetPos(aTargetPos),
		myPath(oPath),
		mySettings(aSettings),
		myCompleteFlag(aCompleteFlag),
		myDebugOpenPoints(aDebugOpenPoints)
	{}

protected:
	ASVONVolume& myVolume;
	UWorld* myWorld;

	SVONLink myStart;
	SVONLink myTarget;
	FVector myStartPos;
	FVector myTargetPos;
	FSVONNavPathSharedPtr* myPath;

	SVONPathFinderSettings mySettings;

	FThreadSafeBool& myCompleteFlag;

	TArray<FVector>& myDebugOpenPoints;

	void DoWork();

	// This next section of code needs to be here.  Not important as to why.

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FSVONFindPathTask, STATGROUP_ThreadPoolAsyncTasks);
	}
};
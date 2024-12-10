#pragma once

#include "UESVON/Public/SVONLink.h"
#include "UESVON/Public/SVONPathFinder.h"
#include "UESVON/Public/SVONTypes.h"

#include <Runtime/Core/Public/Async/AsyncWork.h>
#include <Runtime/Core/Public/HAL/ThreadSafeBool.h>

struct FSVONData;
struct SVONPathFinderSettings;

class FSVONFindPathTask : public FNonAbandonableTask
{
	friend class FAutoDeleteAsyncTask<FSVONFindPathTask>;

public:
	FSVONFindPathTask(const FSVONData& Data, SVONPathFinderSettings& aSettings, UWorld* aWorld, const SVONLink aStart, const SVONLink aTarget, const FVector& aStartPos, const FVector& aTargetPos, FSVONNavPathSharedPtr* oPath, FThreadSafeBool& aCompleteFlag)
		: NavigationData(Data)
		, myWorld(aWorld)
		, myStart(aStart)
		, myTarget(aTarget)
		, myStartPos(aStartPos)
		, myTargetPos(aTargetPos)
		, myPath(oPath)
		, mySettings(aSettings)
		, myCompleteFlag(aCompleteFlag)
	{
	}

protected:
	const FSVONData& NavigationData;
	UWorld* myWorld;

	SVONLink myStart;
	SVONLink myTarget;
	FVector myStartPos;
	FVector myTargetPos;
	FSVONNavPathSharedPtr* myPath;

	// TODO: const this and have the debug data elsewhere
	SVONPathFinderSettings mySettings;

	FThreadSafeBool& myCompleteFlag;

	void DoWork();

	// This next section of code needs to be here.  Not important as to why.

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FSVONFindPathTask, STATGROUP_ThreadPoolAsyncTasks);
	}
};
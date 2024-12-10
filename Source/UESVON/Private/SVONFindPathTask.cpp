#include "UESVON/Public/SVONFindPathTask.h"
#include "UESVON/Public/SVONPathFinder.h"

void FSVONFindPathTask::DoWork()
{
	SVONPathFinder PathFinder(myWorld, NavigationData, mySettings);

	int Result = PathFinder.FindPath(myStart, myTarget, myStartPos, myTargetPos, myPath);

	myCompleteFlag = true;
}

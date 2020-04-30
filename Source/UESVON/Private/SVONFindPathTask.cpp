#include "UESVON/Public/SVONFindPathTask.h"
#include "UESVON/Public/SVONPathFinder.h"

void FSVONFindPathTask::DoWork()
{
	SVONPathFinder pathFinder(myWorld, myVolume, mySettings);

	int result = pathFinder.FindPath(myStart, myTarget, myStartPos, myTargetPos, myPath);

	myCompleteFlag = true;
}

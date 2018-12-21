#include "SVONFindPathTask.h"


#include "SVONPathFinder.h"

void FSVONFindPathTask::DoWork()
{
	SVONPathFinder pathFinder(myWorld, myVolume, mySettings);

	int result = pathFinder.FindPath(myStart, myTarget, myStartPos, myTargetPos, myPath);

	myCompleteFlag = true;
}

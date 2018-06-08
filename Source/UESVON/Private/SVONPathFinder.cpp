#include "SVONPathFinder.h"
#include "SVONLink.h"


bool SVONPathFinder::FindPath(const SVONLink& aStart, const SVONLink& aGoal, SVONPath& oPath)
{
	myOpenSet.Empty();
	myClosedSet.Empty();
	myCameFrom.Empty();
	myFScore.Empty();
	myGScore.Empty();
	myCurrent = SVONLink();
	myGoal = aGoal;


	myOpenSet.Add(aStart);
	myCameFrom.Add(aStart, aStart);
	myGScore.Add(aStart, 0);
	myFScore.Add(aStart, HeuristicScore(aStart, myGoal)); // Distance to target

	while (myOpenSet.Num() > 0)
	{
		
		float lowestScore = FLT_MAX;
		for (SVONLink& link : myOpenSet)
		{
			if (!myFScore.Contains(link) || myFScore[link] < lowestScore)
				myCurrent = link;
		}

		myOpenSet.Remove(myCurrent);
		myClosedSet.Add(myCurrent);

		if (myCurrent == myGoal)
		{
			BuildPath(myCameFrom, oPath);
			return true;
		}

		const SVONNode& currentNode = myVolume.GetNode(myCurrent);

		for (const SVONLink& neighbour : currentNode.myNeighbours)
		{
			ProcessLink(neighbour);
		}
	}

	return false;
}

float SVONPathFinder::HeuristicScore( const SVONLink& aStart, const SVONLink& aTarget)
{
	return DistanceBetween(aStart, aTarget);
}

float SVONPathFinder::DistanceBetween( const SVONLink& aStart, const SVONLink& aTarget)
{
	FVector startPos(0.f), endPos(0.f);
	const SVONNode& startNode = myVolume.GetNode(aStart);
	const SVONNode& endNode = myVolume.GetNode(aTarget);
	myVolume.GetLinkPosition(aStart, startPos);
	myVolume.GetLinkPosition(aTarget, endPos);
	return (startPos - endPos).Size();
}

void SVONPathFinder::ProcessLink(const SVONLink& aNeighbour)
{
	if (aNeighbour.IsValid())
	{
		if (myClosedSet.Contains(aNeighbour))
			return;

		if (!myOpenSet.Contains(aNeighbour))
			myOpenSet.Add(aNeighbour);

		float t_gScore = myGScore.Contains(aNeighbour) ? myGScore[aNeighbour] : FLT_MAX + DistanceBetween(myCurrent, aNeighbour);

		if (t_gScore >= (myGScore.Contains(aNeighbour) ? myGScore[aNeighbour] : FLT_MAX))
			return;

		myCameFrom[aNeighbour] = myCurrent;
		myGScore[aNeighbour] = t_gScore;
		myFScore[aNeighbour] = myGScore[aNeighbour] + HeuristicScore(aNeighbour, myGoal);
	}
}

void SVONPathFinder::BuildPath(TMap<SVONLink, SVONLink>& aCameFrom, SVONPath& oPath)
{
	oPath.ResetPath();
	for (TPair<SVONLink, SVONLink>& step : aCameFrom)
	{
		FVector pos;
		myVolume.GetLinkPosition(step.Key, pos);
		DrawDebugSphere(myVolume.GetWorld(), pos, 100.f, 10, FColor::Cyan, true);
		oPath.AddPoint(pos);
	}

	FVector pos;
	myVolume.GetLinkPosition(myGoal, pos);
	DrawDebugSphere(myVolume.GetWorld(), pos, 100.f, 10, FColor::Cyan, true);
	oPath.AddPoint(pos);


}

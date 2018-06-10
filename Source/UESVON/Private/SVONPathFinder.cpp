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

	int numIterations = 0;

	while (myOpenSet.Num() > 0)
	{
		
		float lowestScore = FLT_MAX;
		for (SVONLink& link : myOpenSet)
		{
			if (!myFScore.Contains(link) || myFScore[link] < lowestScore)
			{
				lowestScore = myFScore[link];
				myCurrent = link;
			}
		}

		myOpenSet.Remove(myCurrent);
		myClosedSet.Add(myCurrent);

		if (myCurrent == myGoal)
		{
			BuildPath(myCameFrom, myCurrent, oPath);
			UE_LOG(UESVON, Display, TEXT("Pathfinding complete, iterations : %i"), numIterations);
			return true;
		}

		const SVONNode& currentNode = myVolume.GetNode(myCurrent);

		if (myCurrent.GetLayerIndex() == 0 && currentNode.myFirstChild.IsValid())
		{
			TArray<SVONLink> leafNeighbours;
			myVolume.GetLeafNeighbours(myCurrent, leafNeighbours);
			for (const SVONLink& neighbour : leafNeighbours)
			{
				ProcessLink(neighbour);
			}

		}
		else
		{
			for (const SVONLink& neighbour : currentNode.myNeighbours)
			{
				ProcessLink(neighbour);
			}
		}



		numIterations++;
	}

	UE_LOG(UESVON, Display, TEXT("Pathfinding failed, iterations : %i"), numIterations);
	return false;
}

float SVONPathFinder::HeuristicScore( const SVONLink& aStart, const SVONLink& aTarget)
{
	return DistanceBetween(aStart, aTarget) * 0.001f;
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
		{
			myOpenSet.Add(aNeighbour);
			if (myDebugOpenNodes)
			{
				FVector pos;
				myVolume.GetLinkPosition(aNeighbour, pos);
				myDebugPoints.Add(pos);
				//DrawDebugSphere(myWorld, pos, 80.f, 10, FColor::White, false, 0.0f, 0, 20.f);
			}
		}
			


		float t_gScore = FLT_MAX;
		if (myGScore.Contains(myCurrent))
			t_gScore = myGScore[myCurrent] + DistanceBetween(myCurrent, aNeighbour);
		else
			myGScore.Add(myCurrent, FLT_MAX);

		if (t_gScore >= (myGScore.Contains(aNeighbour) ? myGScore[aNeighbour] : FLT_MAX))
			return;

		myCameFrom.Add(aNeighbour, myCurrent);
		myGScore.Add(aNeighbour, t_gScore);
		myFScore.Add(aNeighbour, myGScore[aNeighbour] + HeuristicScore(aNeighbour, myGoal));
	}
}

void SVONPathFinder::BuildPath(TMap<SVONLink, SVONLink>& aCameFrom, SVONLink aCurrent, SVONPath& oPath)
{
	
	FVector pos;
	myVolume.GetLinkPosition(myCurrent, pos);
	oPath.AddPoint(pos);

	while (aCameFrom.Contains(aCurrent) && !(aCurrent == aCameFrom[aCurrent]))
	{
		aCurrent = aCameFrom[aCurrent];
		myVolume.GetLinkPosition(aCurrent, pos);
		oPath.AddPoint(pos);
	}

	//myVolume.GetLinkPosition(myGoal, pos);
	//oPath.AddPoint(pos);
	
}

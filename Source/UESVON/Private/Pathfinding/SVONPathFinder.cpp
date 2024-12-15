#include <UESVON/Public/Pathfinding/SVONPathFinder.h>

#include <UESVON/Public/Actor/SVONVolume.h>
#include <UESVON/Public/Data/SVONLink.h>
#include <UESVON/Public/Data/SVONNode.h>
#include <UESVON/Public/Pathfinding/SVONNavigationPath.h>
#include <UESVON/Public/UESVON.h>

int SVONPathFinder::FindPath(const SVONLink& aStart, const SVONLink& aGoal, const FVector& aStartPos, const FVector& aTargetPos, FSVONNavPathSharedPtr* oPath)
{
	myOpenSet.Empty();
	myClosedSet.Empty();
	myCameFrom.Empty();
	myFScore.Empty();
	myGScore.Empty();
	myCurrent = SVONLink();
	myGoal = aGoal;
	myStart = aStart;

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
			BuildPath(myCameFrom, myCurrent, aStartPos, aTargetPos, oPath);
#if WITH_EDITOR
			UE_LOG(UESVON, Display, TEXT("Pathfinding complete, iterations : %i"), numIterations);
#endif
			return 1;
		}

		const SVONNode& currentNode = NavigationData.OctreeData.GetNode(myCurrent);

		TArray<SVONLink> neighbours;

		if (myCurrent.GetLayerIndex() == 0 && currentNode.myFirstChild.IsValid())
		{

			NavigationData.OctreeData.GetLeafNeighbours(myCurrent, neighbours);
		}
		else
		{
			NavigationData.OctreeData.GetNeighbours(myCurrent, neighbours);
		}

		for (const SVONLink& neighbour : neighbours)
		{
			ProcessLink(neighbour);
		}

		numIterations++;
	}
#if WITH_EDITOR
	UE_LOG(UESVON, Display, TEXT("Pathfinding failed, iterations : %i"), numIterations);
#endif
	return 0;
}

float SVONPathFinder::HeuristicScore(const SVONLink& aStart, const SVONLink& aTarget)
{
	/* Just using manhattan distance for now */
	float score = 0.f;

	FVector startPos, endPos;
	NavigationData.GetLinkPosition(aStart, startPos);
	NavigationData.GetLinkPosition(aTarget, endPos);
	switch (mySettings.myPathCostType)
	{
	case ESVONPathCostType::MANHATTAN:
		score = FMath::Abs(endPos.X - startPos.X) + FMath::Abs(endPos.Y - startPos.Y) + FMath::Abs(endPos.Z - startPos.Z);
		break;
	case ESVONPathCostType::EUCLIDEAN:
	default:
		score = (startPos - endPos).Size();
		break;
	}

	score *= (1.0f - (static_cast<float>(aTarget.GetLayerIndex()) / static_cast<float>(NavigationData.OctreeData.GetNumLayers())) * mySettings.myNodeSizeCompensation);

	return score;
}

float SVONPathFinder::GetCost(const SVONLink& aStart, const SVONLink& aTarget)
{
	float cost = 0.f;

	// Unit cost implementation
	if (mySettings.myUseUnitCost)
	{
		cost = mySettings.myUnitCost;
	}
	else
	{

		FVector startPos(0.f), endPos(0.f);
		const SVONNode& startNode = NavigationData.OctreeData.GetNode(aStart);
		const SVONNode& endNode = NavigationData.OctreeData.GetNode(aTarget);
		NavigationData.GetLinkPosition(aStart, startPos);
		NavigationData.GetLinkPosition(aTarget, endPos);
		cost = (startPos - endPos).Size();
	}

	cost *= (1.0f - (static_cast<float>(aTarget.GetLayerIndex()) / static_cast<float>(NavigationData.OctreeData.GetNumLayers())) * mySettings.myNodeSizeCompensation);

	return cost;
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

			if (mySettings.myDebugOpenNodes)
			{
				FVector pos;
				NavigationData.GetLinkPosition(aNeighbour, pos);
				mySettings.myDebugPoints.Add(pos);
			}
		}

		float t_gScore = FLT_MAX;
		if (myGScore.Contains(myCurrent))
			t_gScore = myGScore[myCurrent] + GetCost(myCurrent, aNeighbour);
		else
			myGScore.Add(myCurrent, FLT_MAX);

		if (t_gScore >= (myGScore.Contains(aNeighbour) ? myGScore[aNeighbour] : FLT_MAX))
			return;

		myCameFrom.Add(aNeighbour, myCurrent);
		myGScore.Add(aNeighbour, t_gScore);
		myFScore.Add(aNeighbour, myGScore[aNeighbour] + (mySettings.myEstimateWeight * HeuristicScore(aNeighbour, myGoal)));
	}
}

void SVONPathFinder::BuildPath(TMap<SVONLink, SVONLink>& aCameFrom, SVONLink aCurrent, const FVector& aStartPos, const FVector& aTargetPos, FSVONNavPathSharedPtr* oPath)
{

	FSVONPathPoint pos;

	TArray<FSVONPathPoint> points;

	if (!oPath || !oPath->IsValid())
		return;

	while (aCameFrom.Contains(aCurrent) && !(aCurrent == aCameFrom[aCurrent]))
	{
		aCurrent = aCameFrom[aCurrent];
		NavigationData.GetLinkPosition(aCurrent, pos.myPosition);
		points.Add(pos);
		const SVONNode& node = NavigationData.OctreeData.GetNode(aCurrent);
		// This is rank. I really should sort the layers out
		if (aCurrent.GetLayerIndex() == 0)
		{
			if (!node.HasChildren())
				points[points.Num() - 1].myLayer = 1;
			else
				points[points.Num() - 1].myLayer = 0;
		}
		else
		{
			points[points.Num() - 1].myLayer = aCurrent.GetLayerIndex() + 1;
		}
	}

	if (points.Num() > 1)
	{
		points[0].myPosition = aTargetPos;
		points[points.Num() - 1].myPosition = aStartPos;
	}
	else // If start and end are in the same voxel, just use the start and target positions.
	{
		if (points.Num() == 0)
			points.Emplace();

		points[0].myPosition = aTargetPos;
		points.Emplace(aStartPos, myStart.GetLayerIndex());
	}

	// Smooth_Chaikin(points, mySettings.mySmoothingIterations);

	for (int i = points.Num() - 1; i >= 0; i--)
	{
		oPath->Get()->GetPathPoints().Add(points[i]);
	}
}

// void SVONPathFinder::Smooth_Chaikin(TArray<FVector>& somePoints, int aNumIterations)
//{
//	for (int i = 0; i < aNumIterations; i++)
//	{
//		for (int j = 0; j < somePoints.Num() - 1; j += 2)
//		{
//			FVector start = somePoints[j];
//			FVector end = somePoints[j + 1];
//			if (j > 0)
//				somePoints[j] = FMath::Lerp(start, end, 0.25f);
//			FVector secondVal = FMath::Lerp(start, end, 0.75f);
//			somePoints.Insert(secondVal, j + 1);
//		}
//		somePoints.RemoveAt(somePoints.Num() - 1);
//	}
// }

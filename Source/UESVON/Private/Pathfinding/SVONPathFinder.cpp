#include <UESVON/Public/Pathfinding/SVONPathFinder.h>

#include <UESVON/Public/Actor/SVONVolume.h>
#include <UESVON/Public/Data/SVONLink.h>
#include <UESVON/Public/Data/SVONNode.h>
#include <UESVON/Public/Pathfinding/SVONNavigationPath.h>
#include <UESVON/Public/UESVON.h>

int SVONPathFinder::FindPath(const SVONLink& InStart, const SVONLink& InGoal, const FVector& StartPos, const FVector& TargetPos, FSVONNavPathSharedPtr* Path)
{
	OpenSet.Empty();
	ClosedSet.Empty();
	CameFrom.Empty();
	FScore.Empty();
	GScore.Empty();
	Current = SVONLink();
	Goal = InGoal;
	Start = InStart;

	OpenSet.Add(InStart);
	CameFrom.Add(InStart, InStart);
	GScore.Add(InStart, 0);
	FScore.Add(InStart, HeuristicScore(InStart, InGoal)); // Distance to target

	int numIterations = 0;

	while (OpenSet.Num() > 0)
	{

		float lowestScore = FLT_MAX;
		for (SVONLink& link : OpenSet)
		{
			if (!FScore.Contains(link) || FScore[link] < lowestScore)
			{
				lowestScore = FScore[link];
				Current = link;
			}
		}

		OpenSet.Remove(Current);
		ClosedSet.Add(Current);

		if (Current == Goal)
		{
			BuildPath(CameFrom, Current, StartPos, TargetPos, Path);
#if WITH_EDITOR
			UE_LOG(UESVON, Display, TEXT("Pathfinding complete, iterations : %i"), numIterations);
#endif
			return 1;
		}

		const SVONNode& currentNode = NavigationData.OctreeData.GetNode(Current);

		TArray<SVONLink> neighbours;

		if (Current.GetLayerIndex() == 0 && currentNode.FirstChild.IsValid())
		{

			NavigationData.OctreeData.GetLeafNeighbours(Current, neighbours);
		}
		else
		{
			NavigationData.OctreeData.GetNeighbours(Current, neighbours);
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
	switch (Settings.PathCostType)
	{
	case ESVONPathCostType::MANHATTAN:
		score = FMath::Abs(endPos.X - startPos.X) + FMath::Abs(endPos.Y - startPos.Y) + FMath::Abs(endPos.Z - startPos.Z);
		break;
	case ESVONPathCostType::EUCLIDEAN:
	default:
		score = (startPos - endPos).Size();
		break;
	}

	score *= (1.0f - (static_cast<float>(aTarget.GetLayerIndex()) / static_cast<float>(NavigationData.OctreeData.GetNumLayers())) * Settings.NodeSizeCompensation);

	return score;
}

float SVONPathFinder::GetCost(const SVONLink& aStart, const SVONLink& aTarget)
{
	float cost = 0.f;

	// Unit cost implementation
	if (Settings.UseUnitCost)
	{
		cost = Settings.UnitCost;
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

	cost *= (1.0f - (static_cast<float>(aTarget.GetLayerIndex()) / static_cast<float>(NavigationData.OctreeData.GetNumLayers())) * Settings.NodeSizeCompensation);

	return cost;
}

void SVONPathFinder::ProcessLink(const SVONLink& aNeighbour)
{
	if (aNeighbour.IsValid())
	{
		if (ClosedSet.Contains(aNeighbour))
			return;

		if (!OpenSet.Contains(aNeighbour))
		{
			OpenSet.Add(aNeighbour);

			if (Settings.DebugOpenNodes)
			{
				FVector pos;
				NavigationData.GetLinkPosition(aNeighbour, pos);
				Settings.DebugPoints.Add(pos);
			}
		}

		float t_gScore = FLT_MAX;
		if (GScore.Contains(Current))
			t_gScore = GScore[Current] + GetCost(Current, aNeighbour);
		else
			GScore.Add(Current, FLT_MAX);

		if (t_gScore >= (GScore.Contains(aNeighbour) ? GScore[aNeighbour] : FLT_MAX))
			return;

		CameFrom.Add(aNeighbour, Current);
		GScore.Add(aNeighbour, t_gScore);
		FScore.Add(aNeighbour, GScore[aNeighbour] + (Settings.EstimateWeight * HeuristicScore(aNeighbour, Goal)));
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
		NavigationData.GetLinkPosition(aCurrent, pos.Position);
		points.Add(pos);
		const SVONNode& node = NavigationData.OctreeData.GetNode(aCurrent);
		// This is rank. I really should sort the layers out
		if (aCurrent.GetLayerIndex() == 0)
		{
			if (!node.HasChildren())
				points[points.Num() - 1].Layer = 1;
			else
				points[points.Num() - 1].Layer = 0;
		}
		else
		{
			points[points.Num() - 1].Layer = aCurrent.GetLayerIndex() + 1;
		}
	}

	if (points.Num() > 1)
	{
		points[0].Position = aTargetPos;
		points[points.Num() - 1].Position = aStartPos;
	}
	else // If start and end are in the same voxel, just use the start and target positions.
	{
		if (points.Num() == 0)
			points.Emplace();

		points[0].Position = aTargetPos;
		points.Emplace(aStartPos, Start.GetLayerIndex());
	}

	// Smooth_Chaikin(points, Settings.SmoothingIterations);

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

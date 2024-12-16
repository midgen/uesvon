#pragma once

#include <UESVON/Public/Pathfinding/SVONNavigationPath.h>
#include <UESVON/Public/Data/SVONTypes.h>
#include <UESVON/Public/Data/SVONLink.h>

class ASVONVolume;

struct FNavigationPath;
struct FSVONNavigationPath;
struct SVONLink;

struct SVONPathFinderSettings
{
	bool DebugOpenNodes;
	bool UseUnitCost;
	float UnitCost;
	float EstimateWeight;
	float NodeSizeCompensation;
	int SmoothingIterations;
	ESVONPathCostType PathCostType;
	TArray<FVector> DebugPoints;

	SVONPathFinderSettings()
		: DebugOpenNodes(false)
		, UseUnitCost(false)
		, UnitCost(1.0f)
		, EstimateWeight(1.0f)
		, NodeSizeCompensation(1.0f)
		, SmoothingIterations(0.f)
		, PathCostType(ESVONPathCostType::EUCLIDEAN)
	{
	}
};

class UESVON_API SVONPathFinder
{
public:
	SVONPathFinder(UWorld* World, const FSVONData& Data, SVONPathFinderSettings& Settings)
		: NavigationData(Data)
		, Settings(Settings)
		, World(World){};
	~SVONPathFinder(){};

	/* Performs an A* search from start to target navlink */
	int FindPath(const SVONLink& aStart, const SVONLink& aTarget, const FVector& aStartPos, const FVector& aTargetPos, FSVONNavPathSharedPtr* oPath);

private:
	TArray<SVONLink> OpenSet;
	TSet<SVONLink> ClosedSet;

	TMap<SVONLink, SVONLink> CameFrom;

	TMap<SVONLink, float> GScore;
	TMap<SVONLink, float> FScore;

	SVONLink Start;
	SVONLink Current;
	SVONLink Goal;

	const FSVONData& NavigationData;

	SVONPathFinderSettings& Settings;

	UWorld* World;

	/* A* heuristic calculation */
	float HeuristicScore(const SVONLink& aStart, const SVONLink& aTarget);

	/* Distance between two links */
	float GetCost(const SVONLink& aStart, const SVONLink& aTarget);

	void ProcessLink(const SVONLink& aNeighbour);

	/* Constructs the path by navigating back through our CameFrom map */
	void BuildPath(TMap<SVONLink, SVONLink>& aCameFrom, SVONLink aCurrent, const FVector& aStartPos, const FVector& aTargetPos, FSVONNavPathSharedPtr* oPath);

	/*void Smooth_Chaikin(TArray<FVector>& somePoints, int aNumIterations);*/
};
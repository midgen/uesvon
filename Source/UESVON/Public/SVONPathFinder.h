
#pragma once

#include "UESVON/Public/SVONTypes.h"

class ASVONVolume;

struct FNavigationPath;
struct FSVONNavigationPath;
struct SVONLink;

struct SVONPathFinderSettings
{
	bool myDebugOpenNodes;
	bool myUseUnitCost;
	float myUnitCost;
	float myEstimateWeight;
	float myNodeSizeCompensation;
	int mySmoothingIterations;
	ESVONPathCostType myPathCostType;
	TArray<FVector> myDebugPoints;

	SVONPathFinderSettings()
		: myDebugOpenNodes(false)
		, myUseUnitCost(false)
		, myUnitCost(1.0f)
		, myEstimateWeight(1.0f)
		, myNodeSizeCompensation(1.0f)
		, mySmoothingIterations(0.f)
		, myPathCostType(ESVONPathCostType::EUCLIDEAN)
	{
	}
};

class UESVON_API SVONPathFinder
{
public:
	SVONPathFinder(UWorld* aWorld, const ASVONVolume& aVolume, SVONPathFinderSettings& aSettings)
		: myVolume(aVolume)
		, mySettings(aSettings)
		, myWorld(aWorld){};
	~SVONPathFinder(){};

	/* Performs an A* search from start to target navlink */
	int FindPath(const SVONLink& aStart, const SVONLink& aTarget, const FVector& aStartPos, const FVector& aTargetPos, FSVONNavPathSharedPtr* oPath);

private:
	TArray<SVONLink> myOpenSet;
	TSet<SVONLink> myClosedSet;

	TMap<SVONLink, SVONLink> myCameFrom;

	TMap<SVONLink, float> myGScore;
	TMap<SVONLink, float> myFScore;

	SVONLink myStart;
	SVONLink myCurrent;
	SVONLink myGoal;

	const ASVONVolume& myVolume;

	SVONPathFinderSettings& mySettings;

	UWorld* myWorld;

	/* A* heuristic calculation */
	float HeuristicScore(const SVONLink& aStart, const SVONLink& aTarget);

	/* Distance between two links */
	float GetCost(const SVONLink& aStart, const SVONLink& aTarget);

	void ProcessLink(const SVONLink& aNeighbour);

	/* Constructs the path by navigating back through our CameFrom map */
	void BuildPath(TMap<SVONLink, SVONLink>& aCameFrom, SVONLink aCurrent, const FVector& aStartPos, const FVector& aTargetPos, FSVONNavPathSharedPtr* oPath);

	/*void Smooth_Chaikin(TArray<FVector>& somePoints, int aNumIterations);*/
};
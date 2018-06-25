
#pragma once
#include "CoreMinimal.h"


struct SVONPath;
class ASVONVolume;
struct SVONLink;

struct FNavigationPath;

class UESVON_API SVONPathFinder
{
public:
	SVONPathFinder(const ASVONVolume& aVolume, bool aDebugOpenNodes, UWorld* aWorld, TArray<FVector>& aDebugPoints)
		: myVolume(aVolume), 
		myDebugOpenNodes (aDebugOpenNodes),
		myWorld(aWorld),
		myDebugPoints(aDebugPoints) {};
	~SVONPathFinder() {};

	/* Performs an A* search from start to target navlink */
	int FindPath(const SVONLink& aStart, const SVONLink& aTarget, FNavPathSharedPtr* oPath);

	const SVONPath& GetPath() const { return myPath; }
	const FNavigationPath& GetNavPath();  

private:
	SVONPath myPath;

	FNavigationPath myNavPath;

	TArray<SVONLink> myOpenSet;
	TSet<SVONLink> myClosedSet;

	TMap<SVONLink, SVONLink> myCameFrom;

	TMap<SVONLink, float>    myGScore;
	TMap<SVONLink, float>    myFScore;

	SVONLink myCurrent;
	SVONLink myGoal;

	const ASVONVolume& myVolume;

	TArray<FVector>& myDebugPoints;

	bool myDebugOpenNodes;
	UWorld* myWorld;

	/* A* heuristic calculation */
	float HeuristicScore(const SVONLink& aStart, const SVONLink& aTarget);

	/* Distance between two links */
	float DistanceBetween(const SVONLink& aStart, const SVONLink& aTarget);

	void ProcessLink(const SVONLink& aNeighbour);

	/* Constructs the path by navigating back through our CameFrom map */
	void BuildPath(TMap<SVONLink, SVONLink>& aCameFrom, SVONLink aCurrent, FNavPathSharedPtr* oPath);

};
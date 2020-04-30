#pragma once

UENUM(BlueprintType)
enum class ESVONPathCostType : uint8
{
	MANHATTAN,
	EUCLIDEAN
};

struct FNavigationPath;
class ASVONVolume;

struct UESVON_API FSVONPathPoint
{
	FSVONPathPoint()
		: myPosition(FVector())
		, myLayer(-1)
	{
	}
	FSVONPathPoint(const FVector& aPosition, int aLayer)
		: myPosition(aPosition)
		, myLayer(aLayer)
	{
	}

	FVector myPosition; // Position of the point
	int myLayer;		// Layer that the point came from (so we can infer it's volume)
};

struct UESVON_API FSVONNavigationPath
{

public:
	void AddPoint(const FSVONPathPoint& aPoint);
	void ResetForRepath();

	void DebugDraw(UWorld* aWorld, const ASVONVolume& aVolume);

	const TArray<FSVONPathPoint>& GetPathPoints() const
	{
		return myPoints;
	};

	TArray<FSVONPathPoint>& GetPathPoints() { return myPoints; }

	bool IsReady() const { return myIsReady; };
	void SetIsReady(bool aIsReady) { myIsReady = aIsReady; }

	// Copy the path positions into a standard navigation path
	void CreateNavPath(FNavigationPath& aOutPath);

protected:
	bool myIsReady;
	TArray<FSVONPathPoint> myPoints;
};
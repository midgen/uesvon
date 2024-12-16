#pragma once

struct FSVONData;

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
		: Position(FVector())
		, Layer(-1)
	{
	}
	FSVONPathPoint(const FVector& aPosition, int aLayer)
		: Position(aPosition)
		, Layer(aLayer)
	{
	}

	FVector Position; // Position of the point
	int Layer;		// Layer that the point came from (so we can infer it's volume)
};

struct UESVON_API FSVONNavigationPath
{

public:
	void AddPoint(const FSVONPathPoint& aPoint);
	void ResetForRepath();

	void DebugDraw(UWorld* World, const FSVONData& Data);

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
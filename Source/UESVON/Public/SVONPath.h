#pragma once
#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class ESVONPathCostType : uint8
{
	MANHATTAN,
	EUCLIDEAN
};

struct UESVON_API SVONPath
{
protected:
	TArray<FVector> myPoints;

public:
	void AddPoint(const FVector& aPoint);
	void ResetPath();

	void DebugDraw(UWorld* aWorld);

	const TArray<FVector>& GetPoints() const {
		return myPoints;
	};
};
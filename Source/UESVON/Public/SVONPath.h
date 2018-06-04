#pragma once
#include "CoreMinimal.h"

struct UESVON_API SVONPath
{
protected:
	TArray<FVector> myPoints;

public:
	void AddPoint(const FVector& aPoint);
	void ResetPath();

	const TArray<FVector>& GetPoints() const {
		return myPoints;
	};
};
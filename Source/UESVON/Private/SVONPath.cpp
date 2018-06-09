
#include "SVONPath.h"

void SVONPath::AddPoint(const FVector& aPoint)
{
	myPoints.Add(aPoint);
}

void SVONPath::ResetPath()
{
	myPoints.Empty();
}

void SVONPath::DebugDraw(UWorld* aWorld)
{
	for (int i = 0; i < myPoints.Num(); i++)
	{
		FVector& point = myPoints[i];
		
		
		if (i > 0)
		{
			DrawDebugSphere(aWorld, point, 50.f, 20, FColor::Cyan, false, 0.0f, 0, 20.f);
			DrawDebugLine(aWorld, point, myPoints[i - 1], FColor::Cyan, false, 0.f, 0, 20.f);
		}
	}
}


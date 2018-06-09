
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
		
		
		if (i < myPoints.Num() - 1)
		{
			FVector offSet(0.f);
			if (i == 0)
				//offSet.Z -= 300.f;
			
			DrawDebugSphere(aWorld, point + offSet, 30.f, 20, FColor::Cyan, false, 0.0f, 0, 20.f);

			DrawDebugLine(aWorld, point + offSet, myPoints[i+1], FColor::Cyan, false, 0.f, 0, 20.f);
		}
	}
}


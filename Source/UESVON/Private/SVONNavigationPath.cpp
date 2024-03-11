
#include "UESVON/Public/SVONNavigationPath.h"
#include "UESVON/Public/SVONVolume.h"

#include <Runtime/Engine/Classes/Debug/DebugDrawService.h>
#include <Runtime/Engine/Public/DrawDebugHelpers.h>

#include <Runtime/NavigationSystem/Public/NavigationData.h>

void FSVONNavigationPath::AddPoint(const FSVONPathPoint& aPoint)
{
	myPoints.Add(aPoint);
}

void FSVONNavigationPath::ResetForRepath()
{
	myPoints.Empty();
}

void FSVONNavigationPath::DebugDraw(UWorld* aWorld, const ASVONVolume& aVolume)
{
	for (int i = 0; i < myPoints.Num(); i++)
	{
		FSVONPathPoint& point = myPoints[i];

		if (i < myPoints.Num() - 1)
		{
			FVector offSet(0.f);
			//if (i == 0)
			//offSet.Z -= 300.f;
			float size = point.myLayer == 0 ? aVolume.GetVoxelSize(point.myLayer) * 0.25f : aVolume.GetVoxelSize(point.myLayer) * 0.5f;

			DrawDebugBox(aWorld, point.myPosition, FVector(size), SVONStatics::myLinkColors[point.myLayer], true, -1.f, 0, 30.f);

			DrawDebugSphere(aWorld, point.myPosition + offSet, 30.f, 20, FColor::Cyan, true, -1.f, 0, 100.f);

			//DrawDebugLine(aWorld, point.myPosition + offSet, myPoints[i+1].myPosition, FColor::Cyan, true, -1.f, 0, 100.f);
		}
	}
}

void FSVONNavigationPath::CreateNavPath(FNavigationPath& aOutPath)
{
	for (const FSVONPathPoint& point : myPoints)
	{
		aOutPath.GetPathPoints().Add(point.myPosition);
	}
}

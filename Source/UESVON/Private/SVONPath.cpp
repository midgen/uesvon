
#include "SVONPath.h"

void SVONPath::AddPoint(const FVector& aPoint)
{
	myPoints.Add(aPoint);
}

void SVONPath::ResetPath()
{
	myPoints.Empty();
}


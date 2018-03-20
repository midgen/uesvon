#pragma once

#include "CoreMinimal.h"
#include "SVONLink.h"

struct UESVON_API SVONNode
{
	FVector myPosition;
	
	SVONLink myParent;
	SVONLink myFirstChild;

	SVONLink myNeighbours[6];

	SVONNode() 
	{ 
		myPosition = FVector(0.0f); 
	}
};
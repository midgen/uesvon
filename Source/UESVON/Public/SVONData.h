#pragma once

#include "CoreMinimal.h"
#include "SVONNode.h"
#include "SVONLeafNode.h"

struct SVONData
{
	// SVO data
	TArray<TArray<SVONNode>> myLayers;
	TArray<SVONLeafNode> myLeafNodes;
};
#pragma once

#include "CoreMinimal.h"

struct UESVON_API SVONLink
{
	// We'll pack this into a single uint32 later

	uint8 myLayerIndex;
	uint_fast32_t myNodeIndex;
	uint8 mySubnodeIndex;

	SVONLink() : myLayerIndex(0),
		myNodeIndex(0),
		mySubnodeIndex(0) {}

	uint8 GetLayerIndex() const { return myLayerIndex; }
	void SetLayerIndex(const uint8 aLayerIndex) { myLayerIndex = aLayerIndex; }

	uint_fast32_t GetNodeIndex() const { return myNodeIndex; }
	void SetLayerIndex(const uint_fast32_t aNodeIndex) { myNodeIndex = aNodeIndex; }

	uint8 GetSubnodeIndex() const { return mySubnodeIndex; }
	void SetSubnodeIndex(const uint8 aSubnodeIndex) { mySubnodeIndex = aSubnodeIndex; }

};
#pragma once

#include "CoreMinimal.h"

struct UESVON_API SVONLink
{
	unsigned int myLayerIndex:4;
	unsigned int myNodeIndex:22;
	unsigned int mySubnodeIndex:6;

	SVONLink() : 
		myLayerIndex(16),
		myNodeIndex(0),
		mySubnodeIndex(0) {}

	SVONLink(uint8 aLayer, uint_fast32_t aNodeIndex, uint8 aSubNodeIndex)
		: myLayerIndex(aLayer),
		myNodeIndex(aNodeIndex),
		mySubnodeIndex(aSubNodeIndex) {}

	uint8 GetLayerIndex() const { return myLayerIndex; }
	void SetLayerIndex(const uint8 aLayerIndex) { myLayerIndex = aLayerIndex; }

	uint_fast32_t GetNodeIndex() const { return myNodeIndex; }
	void SetNodeIndex(const uint_fast32_t aNodeIndex) { myNodeIndex = aNodeIndex; }

	uint8 GetSubnodeIndex() const { return mySubnodeIndex; }
	void SetSubnodeIndex(const uint8 aSubnodeIndex) { mySubnodeIndex = aSubnodeIndex; }

	bool IsValid() const { return myLayerIndex != 16; }
	void SetInvalid() { myLayerIndex = 16; }

	bool IsLeafNode() const { return myLayerIndex == 15; }
	void SetLeafNode() { myLayerIndex = 15; }

	static SVONLink GetInvalidLink() { return SVONLink(16, 0, 0); }

	FString ToString() 
	{
		return FString::Printf(TEXT("%i:%i:%i"), myLayerIndex, myNodeIndex, mySubnodeIndex);
	};

};


#pragma once

#include "UESVON/Private/libmorton/morton.h"
#include "UESVON/Public/SVONDefines.h"

struct UESVON_API SVONLeafNode
{
	uint_fast64_t myVoxelGrid = 0;

	inline bool GetNodeAt(uint_fast32_t aX, uint_fast32_t aY, uint_fast32_t aZ) const
	{
		uint_fast64_t index = morton3D_64_encode(aX, aY, aZ);
		return (myVoxelGrid & (1ULL << index)) != 0;
	}

	inline void SetNodeAt(uint_fast32_t aX, uint_fast32_t aY, uint_fast32_t aZ)
	{
		uint_fast64_t index = morton3D_64_encode(aX, aY, aZ);
		myVoxelGrid |= 1ULL << index;
	}

	inline void SetNode(uint8 aIndex)
	{
		myVoxelGrid |= 1ULL << aIndex;
	}

	inline bool GetNode(mortoncode_t aIndex) const
	{
		return (myVoxelGrid & (1ULL << aIndex)) != 0;
	}

	inline bool IsCompletelyBlocked() const
	{
		return myVoxelGrid == -1;
	}

	inline bool IsEmpty() const
	{
		return myVoxelGrid == 0;
	}
};

FORCEINLINE FArchive& operator<<(FArchive& Ar, SVONLeafNode& aSVONLeafNode)
{
	Ar << aSVONLeafNode.myVoxelGrid;
	return Ar;
}

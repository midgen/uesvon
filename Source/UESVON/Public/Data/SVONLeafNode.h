#pragma once

#include <UESVON/Private/Library/libmorton/morton.h>
#include <UESVON/Public/Data/SVONDefines.h>

struct UESVON_API SVONLeafNode
{
	uint_fast64_t VoxelGrid = 0;

	inline bool GetNodeAt(uint_fast32_t aX, uint_fast32_t aY, uint_fast32_t aZ) const
	{
		uint_fast64_t index = morton3D_64_encode(aX, aY, aZ);
		return (VoxelGrid & (1ULL << index)) != 0;
	}

	inline void SetNodeAt(uint_fast32_t aX, uint_fast32_t aY, uint_fast32_t aZ)
	{
		uint_fast64_t index = morton3D_64_encode(aX, aY, aZ);
		VoxelGrid |= 1ULL << index;
	}

	inline void SetNode(uint8 aIndex)
	{
		VoxelGrid |= 1ULL << aIndex;
	}

	inline bool GetNode(mortoncode_t aIndex) const
	{
		return (VoxelGrid & (1ULL << aIndex)) != 0;
	}

	inline bool IsCompletelyBlocked() const
	{
		return VoxelGrid == -1;
	}

	inline bool IsEmpty() const
	{
		return VoxelGrid == 0;
	}
};

FORCEINLINE FArchive& operator<<(FArchive& Ar, SVONLeafNode& aSVONLeafNode)
{
	Ar << aSVONLeafNode.VoxelGrid;
	return Ar;
}

#pragma once

struct UESVON_API SVONLink
{
	unsigned int LayerIndex:4;
	unsigned int NodeIndex:22;
	unsigned int SubnodeIndex:6;

	SVONLink() : 
		LayerIndex(15),
		NodeIndex(0),
		SubnodeIndex(0) {}

	SVONLink(uint8 aLayer, uint_fast32_t aNodeIndex, uint8 aSubNodeIndex)
		: LayerIndex(aLayer),
		NodeIndex(aNodeIndex),
		SubnodeIndex(aSubNodeIndex) {}

	uint8 GetLayerIndex() const { return LayerIndex; }
	void SetLayerIndex(const uint8 aLayerIndex) { LayerIndex = aLayerIndex; }

	uint_fast32_t GetNodeIndex() const { return NodeIndex; }
	void SetNodeIndex(const uint_fast32_t aNodeIndex) { NodeIndex = aNodeIndex; }

	uint8 GetSubnodeIndex() const { return SubnodeIndex; }
	void SetSubnodeIndex(const uint8 aSubnodeIndex) { SubnodeIndex = aSubnodeIndex; }

	bool IsValid() const { return LayerIndex != 15; }
	void SetInvalid() { LayerIndex = 15; }

	bool operator==(const SVONLink& aOther) const {
		return memcmp(this, &aOther, sizeof(SVONLink)) == 0;
	}

	static SVONLink GetInvalidLink() { return SVONLink(15, 0, 0); }

	FString ToString() 
	{
		return FString::Printf(TEXT("%i:%i:%i"), LayerIndex, NodeIndex, SubnodeIndex);
	};

};

FORCEINLINE uint32 GetTypeHash(const SVONLink& b)
{
	return *(uint32*)&b;
}


FORCEINLINE FArchive &operator <<(FArchive &Ar, SVONLink& aSVONLink)
{
	Ar.Serialize(&aSVONLink, sizeof(SVONLink));
	return Ar;
}
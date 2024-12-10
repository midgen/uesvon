#pragma once

class ASVONVolume;
struct SVONLink;

class SVONMediator
{
public:
	static bool GetLinkFromPosition(const FVector& aPosition, const ASVONVolume& aVolume, SVONLink& oLink);

	static void GetVolumeXYZ(const FVector& aPosition, const ASVONVolume& aVolume, const int aLayer, FIntVector& oXYZ);
};
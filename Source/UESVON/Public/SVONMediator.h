#pragma once

class ASVONVolume;
struct SVONLink;

class UESVON_API SVONMediator
{
public:
	static bool GetLinkFromPosition(const FVector& aPosition, const ASVONVolume& aVolume, SVONLink& oLink);

	static void GetVolumeXYZ(const FVector& aPosition, const ASVONVolume& aVolume, const int aLayer, FIntVector& oXYZ);

	static bool FindPath(ASVONVolume& aVolume, const SVONLink& aStart, const SVONLink& aTarget, TMap<SVONLink, SVONLink>& oPath);

	static float HeuristicScore(const ASVONVolume& aVolume, const SVONLink& aStart, const SVONLink& aTarget);

	static float DistanceBetween(const ASVONVolume& aVolume, const SVONLink& aStart, const SVONLink& aTarget);
};
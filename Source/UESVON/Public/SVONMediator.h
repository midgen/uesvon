#pragma once

class UESVON_API SVONMediator
{
	static bool GetLinkFromPosition(const FVector& aPosition, const ASVONVolume& aVolume, SVONLink& oLink);
};
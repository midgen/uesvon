#include "SVONDefines.h"

const FIntVector SVONStatics::dirs[] = {
	FIntVector(1,0,0),
	FIntVector(-1,0,0),
	FIntVector(0,1,0),
	FIntVector(0,-1,0),
	FIntVector(0,0,1),
	FIntVector(0,0,-1)
};

const FColor SVONStatics::myLayerColors[] = { FColor::Orange, FColor::Yellow, FColor::White, FColor::Blue, FColor::Turquoise, FColor::Cyan, FColor::Emerald, FColor::Orange };

const FColor SVONStatics::myLinkColors[] = { FColor(0xFF000000), FColor(0xFF444444),FColor(0xFF888888), FColor(0xFFBBBBBB), FColor(0xFFFFFFFF), FColor(0xFF999999), FColor(0xFF777777), FColor(0xFF555555) };
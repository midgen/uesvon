#pragma once

#include "CoreMinimal.h"

typedef uint8 layerindex_t;
typedef int32 nodeindex_t;
typedef uint8 subnodeindex_t;
typedef uint_fast64_t mortoncode_t;
typedef uint_fast32_t posint_t;

UENUM(BlueprintType)		
enum class EBuildTrigger : uint8
{
	OnEdit	UMETA(DisplayName = "On Edit"),
	Manual 	UMETA(DisplayName = "Manual")
};

enum class dir : uint8
{
	pX, nX, pY, nY, pZ, nZ
};

#define LEAF_LAYER_INDEX 14;

class UESVON_API SVONStatics
{
public:
	static const FIntVector dirs[];
	static const FColor myLayerColors[];
	static const FColor myLinkColors[];

};
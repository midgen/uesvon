#pragma once

#include "CoreMinimal.h"

typedef uint8 layerindex;
typedef int32 nodeindex;
typedef uint8 subnodeindex;
typedef uint_fast64_t mortoncode;
typedef uint_fast32_t posint;

UENUM(BlueprintType)		
enum class EBuildTrigger : uint8
{
	OnEdit	UMETA(DisplayName = "On Edit"),
	Manual 	UMETA(DisplayName = "Manual")
};


class UESVON_API SVONStatics
{
public:
	static const FIntVector dirs[];
	static const FColor myLayerColors[];
	static const FColor myLinkColors[];

};
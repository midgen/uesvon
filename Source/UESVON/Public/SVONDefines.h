#pragma once

#include <Runtime/AIModule/Classes/AITypes.h>

typedef uint8 layerindex_t;
typedef int32 nodeindex_t;
typedef uint8 subnodeindex_t;
typedef uint_fast64_t mortoncode_t;
typedef uint_fast32_t posint_t;

UENUM(BlueprintType)
enum class EBuildTrigger : uint8
{
	OnEdit UMETA(DisplayName = "On Edit"),
	Manual UMETA(DisplayName = "Manual")
};

enum class dir : uint8
{
	pX,
	nX,
	pY,
	nY,
	pZ,
	nZ
};

#define LEAF_LAYER_INDEX 14;

class UESVON_API SVONStatics
{
public:
	static const FIntVector dirs[];
	static const nodeindex_t dirChildOffsets[6][4];
	static const nodeindex_t dirLeafChildOffsets[6][16];
	static const FColor myLayerColors[];
	static const FColor myLinkColors[];
};

UENUM(BlueprintType)
namespace ESVONPathfindingRequestResult
{
enum Type
{
	Failed,		   // Something went wrong
	ReadyToPath,   // Pre-reqs satisfied
	AlreadyAtGoal, // No need to move
	Deferred,	   // Passed request to another thread, need to wait
	Success		   // it worked!
};
}

struct UESVON_API FSVONPathfindingRequestResult
{
	FAIRequestID MoveId;
	TEnumAsByte<ESVONPathfindingRequestResult::Type> Code;

	FSVONPathfindingRequestResult()
		: MoveId(FAIRequestID::InvalidRequest)
		, Code(ESVONPathfindingRequestResult::Failed)
	{
	}
	operator ESVONPathfindingRequestResult::Type() const { return Code; }
};
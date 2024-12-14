#pragma once

#include "SVONCollisionQueryInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class USVONCollisionQueryInterface : public UInterface
{
	GENERATED_BODY()
};

class ISVONCollisionQueryInterface
{
	GENERATED_BODY()

public:
	/** Add interface function declarations here */
	virtual bool IsBlocked(const FVector& Position, const float VoxelSize, ECollisionChannel CollisionChannel, const float AgentRadius) const = 0;
};
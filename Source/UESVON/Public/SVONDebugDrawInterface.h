#pragma once

#include "SVONDebugDrawInterface.generated.h"

UINTERFACE(MinimalAPI)
class USVONDebugDrawInterface : public UInterface
{
	GENERATED_BODY()
};

class ISVONDebugDrawInterface
{
	GENERATED_BODY()

public:
	/** Add interface function declarations here */
	
	virtual void SVONDrawDebugString(const FVector& Position, const FString& String, const FColor& Color) const = 0;
	virtual void SVONDrawDebugBox(const FVector& Position, const float Size, const FColor& Color) const = 0;
};
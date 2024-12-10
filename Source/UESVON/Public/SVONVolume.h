#pragma once

#include <UESVON/Public/SVONData.h>
#include <UESVON/Public/SVONCollisionQueryInterface.h>
#include <UESVON/Public/SVONDebugDrawInterface.h>

#include <Runtime/Engine/Classes/GameFramework/Volume.h>

#include "SVONVolume.generated.h"

/**
 *  SVONVolume contains the navigation data for the volume, and the methods for generating that data
		See SVONMediator for public query functions
 */
UCLASS(hidecategories = (Tags, Cooking, Actor, HLOD, Mobile, LOD))
class UESVON_API ASVONVolume : public AVolume, public ISVONCollisionQueryInterface, public ISVONDebugDrawInterface
{
	GENERATED_BODY()

public:

	ASVONVolume(const FObjectInitializer& ObjectInitializer);

	//~ Begin AActor Interface
	void BeginPlay() override;
	bool ShouldTickIfViewportsOnly() const override { return true; }
	//~ End AActor Interface

#if WITH_EDITOR
	//~ Begin UObject Interface
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	void PostEditUndo() override;
	//~ End UObject Interface
#endif // WITH_EDITOR 

	//~ Begin UObject 
	void Serialize(FArchive& Ar) override;
	//~ End UObject 

	bool Generate();
	void ClearData();

	const FSVONData& GetNavData() const { return NavigationData; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation")
	FSVONGenerationParameters GenerationParameters;

	bool bIsReadyForNavigation;

private:
	FSVONData NavigationData;

	void UpdateBounds();

// Inherited via ISVONCollisionQueryInterface
	bool IsBlocked(const FVector& Position, const float VoxelSize, ECollisionChannel CollisionChannel, const float AgentRadius) const override;

// Inherited via ISVONDebugDrawInterface
	void SVONDrawDebugString(const FVector& Position, const FString& String, const FColor& Color) const override;
	void SVONDrawDebugBox(const FVector& Position, const float Size, const FColor& Color) const override;
};

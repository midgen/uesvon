#pragma once

#include <UESVON/Public/Data/SVONData.h>
#include <UESVON/Public/Interface/SVONDebugDrawInterface.h>
#include <UESVON/Public/Interface/SVONSubsystemInterface.h>

#include <Runtime/Engine/Classes/GameFramework/Volume.h>

#include "SVONVolume.generated.h"

/**
 *  SVONVolume contains the navigation data for the volume
 */
UCLASS(hidecategories = (Tags, Cooking, Actor, HLOD, Mobile, LOD))
class UESVON_API ASVONVolume : public AVolume, public ISVONDebugDrawInterface
{
	GENERATED_BODY()

public:

	ASVONVolume(const FObjectInitializer& ObjectInitializer);

	//~ Begin AActor Interface
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
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
	bool HasData() const;
	void ClearData();

	const FSVONData& GetNavData() const { return NavigationData; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation")
	FSVONGenerationParameters GenerationParameters;

	bool bIsReadyForNavigation{false};

private:
	FSVONData NavigationData;

	UPROPERTY()
	TScriptInterface<ISVONSubsystemInterface> SVONSubsystemInterface;
	UPROPERTY()
	TScriptInterface<ISVONCollisionQueryInterface> CollisionQueryInterface;

	void UpdateBounds();

//  ISVONDebugDrawInterface BEGIN
	void SVONDrawDebugString(const FVector& Position, const FString& String, const FColor& Color) const override;
	void SVONDrawDebugBox(const FVector& Position, const float Size, const FColor& Color) const override;
//  ISVONDebugDrawInterface END
};

#pragma once

#include <UESVON/Public/Data/SVONLink.h>
#include <UESVON/Public/Pathfinding/SVONNavigationPath.h>
#include <UESVON/Public/Data/SVONTypes.h>

#include <Runtime/Engine/Classes/Components/ActorComponent.h>

#include "SVONNavigationComponent.generated.h"

class ASVONVolume;
struct SVONLink;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UESVON_API USVONNavigationComponent : public UActorComponent
{
	GENERATED_BODY()

	USVONNavigationComponent(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation | Debug")
	bool bDebugPrintCurrentPosition;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation | Debug")
	bool bDebugPrintMortonCodes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation | Debug")
	bool bDebugDrawOpenNodes = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation | Heuristics")
	bool bUseUnitCost = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation | Heuristics")
	float UnitCost = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation | Heuristics")
	float EstimateWeight = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation | Heuristics")
	float NodeSizeCompensation = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation | Heuristics")
	ESVONPathCostType PathCostType = ESVONPathCostType::EUCLIDEAN;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation | Smoothing")
	int SmoothingIterations = 0;

	const ASVONVolume* GetCurrentVolume() const { return CurrentNavVolume; }

	SVONLink GetNavPosition() const;
	virtual FVector GetPawnPosition() const;
	FSVONNavPathSharedPtr& GetPath() { return CurrentPath; }
	void SetCurrentNavVolume(const ASVONVolume* Volume);

	bool FindPathAsync(const FVector& aStartPosition, const FVector& aTargetPosition, FThreadSafeBool& aCompleteFlag, FSVONNavPathSharedPtr* oNavPath);
	bool FindPathImmediate(const FVector& aStartPosition, const FVector& aTargetPosition, FSVONNavPathSharedPtr* oNavPath);
	UFUNCTION(BlueprintCallable, Category = UESVON)
	void FindPathImmediate(const FVector& aStartPosition, const FVector& aTargetPosition, TArray<FVector>& OutPathPoints);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// The current navigation volume that the owned pawn is inside, null if not inside a volume
	UPROPERTY()
	const ASVONVolume* CurrentNavVolume;

	bool HasNavData() const;

	// Print current layer/morton code information
	void DebugLocalPosition(FVector& aPosition);

	FSVONNavPathSharedPtr CurrentPath;
};

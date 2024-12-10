#pragma once

#include <Runtime/CoreUObject/Public/UObject/ObjectMacros.h>
#include "SVONGenerationParameters.generated.h"

UENUM(BlueprintType)
enum class ESVOGenerationStrategy : uint8
{
	UseBaked UMETA(DisplayName = "Use Baked"),
	GenerateOnBeginPlay UMETA(DisplayName = "Generate OnBeginPlay")
};

USTRUCT(BlueprintType)
struct UESVON_API FSVONGenerationParameters
{
	GENERATED_BODY()

	// Debug Parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation")
	float DebugDistance{5000.f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation")
	bool ShowVoxels{false};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation")
	bool ShowLeafVoxels{false};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation")
	bool ShowMortonCodes{false};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation")
	bool ShowNeighbourLinks{false};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation")
	bool ShowParentChildLinks{false};

	// Generation parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation")
	int32 VoxelPower{3};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation")
	TEnumAsByte<ECollisionChannel> CollisionChannel{ECollisionChannel::ECC_MAX};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation")
	float Clearance = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SVO Navigation")
	ESVOGenerationStrategy GenerationStrategy = ESVOGenerationStrategy::UseBaked;

	// Transient data used during generation
	FVector Origin{FVector::ZeroVector};
	FVector Extents{FVector::ZeroVector};
	FVector DebugPosition{FVector::ZeroVector};
};
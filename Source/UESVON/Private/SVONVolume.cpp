#include "UESVON/Public/SVONVolume.h"

#include <UESVON/Public/Subsystem/SVONSubsystem.h>

#include <Runtime/Engine/Classes/Components/BrushComponent.h>
#include <Runtime/Engine/Classes/Components/LineBatchComponent.h>
#include <Runtime/Engine/Classes/Engine/CollisionProfile.h>
#include <Runtime/Engine/Classes/GameFramework/PlayerController.h>
#include <Runtime/Engine/Public/DrawDebugHelpers.h>

#include <chrono>

using namespace std::chrono;

ASVONVolume::ASVONVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GetBrushComponent()->Mobility = EComponentMobility::Static;
	BrushColor = FColor(255, 255, 255, 255);
	bColored = true;
	UpdateBounds();
}

#if WITH_EDITOR

void ASVONVolume::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void ASVONVolume::PostEditUndo()
{
	Super::PostEditUndo();
}

#endif // WITH_EDITOR

// Regenerates the SVO Navigation Data
bool ASVONVolume::Generate()
{
	// Reset nav data
	NavigationData.ResetForGeneration();
	// Update parameters
	NavigationData.UpdateGenerationParameters(GenerationParameters);

#if WITH_EDITOR
	// Needed for debug rendering
	GetWorld()->PersistentLineBatcher->SetComponentTickEnabled(false);

	// If we're running the game, use the first player controller position for debugging
	APlayerController* pc = GetWorld()->GetFirstPlayerController();
	if (pc)
	{
		NavigationData.SetDebugPosition(pc->GetPawn()->GetActorLocation());
	}
	// otherwise, use the viewport camera location if we're just in the editor
	else if (GetWorld()->ViewLocationsRenderedLastFrame.Num() > 0)
	{
		NavigationData.SetDebugPosition(GetWorld()->ViewLocationsRenderedLastFrame[0]);
	}

	FlushPersistentDebugLines(GetWorld());

	// Setup timing
	milliseconds startMs = duration_cast<milliseconds>(
		system_clock::now().time_since_epoch());

#endif // WITH_EDITOR

	UpdateBounds();
	NavigationData.Generate(*GetWorld(), *this, *this);

#if WITH_EDITOR

	int32 BuildTime = (duration_cast<milliseconds>(
						   system_clock::now().time_since_epoch()) -
					   startMs)
						  .count();

	int32 TotalNodes = 0;

	for (int i = 0; i < NavigationData.OctreeData.GetNumLayers(); i++)
	{
		TotalNodes += NavigationData.OctreeData.Layers[i].Num();
	}

	int32 TotalBytes = sizeof(SVONNode) * TotalNodes;
	TotalBytes += sizeof(SVONLeafNode) * NavigationData.OctreeData.LeafNodes.Num();

	UE_LOG(UESVON, Display, TEXT("Generation Time : %d"), BuildTime);
	UE_LOG(UESVON, Display, TEXT("Total Layers-Nodes : %d-%d"), NavigationData.OctreeData.GetNumLayers(), TotalNodes);
	UE_LOG(UESVON, Display, TEXT("Total Leaf Nodes : %d"), NavigationData.OctreeData.LeafNodes.Num());
	UE_LOG(UESVON, Display, TEXT("Total Size (bytes): %d"), TotalBytes);
#endif

	return true;
}

bool ASVONVolume::HasData() const
{
	return NavigationData.OctreeData.GetNumLayers() > 0;
}

void ASVONVolume::UpdateBounds()
{
	FVector Origin, Extent;
	FBox Bounds = GetComponentsBoundingBox(true);
	Bounds.GetCenterAndExtents(Origin, Extent);

	NavigationData.SetExtents(Origin, Extent);
}

bool ASVONVolume::IsBlocked(const FVector& Position, const float VoxelSize, ECollisionChannel CollisionChannel, const float AgentRadius) const
{
	FCollisionQueryParams Params;
	Params.bFindInitialOverlaps = true;
	Params.bTraceComplex = false;
	Params.TraceTag = "SVONLeafRasterize";

	return GetWorld()->OverlapBlockingTestByChannel(Position, FQuat::Identity, CollisionChannel, FCollisionShape::MakeBox(FVector(VoxelSize + AgentRadius)), Params);
}

void ASVONVolume::SVONDrawDebugString(const FVector& Position, const FString& String, const FColor& Color) const
{
	DrawDebugString(GetWorld(), Position, String, nullptr, Color, -1, false);
}

void ASVONVolume::SVONDrawDebugBox(const FVector& Position, const float Size, const FColor& Color) const
{
	DrawDebugBox(GetWorld(), Position, FVector(Size), FQuat::Identity, Color, true, -1.f, 0, .0f);
}

void ASVONVolume::ClearData()
{
	NavigationData.ResetForGeneration();
}

void ASVONVolume::Serialize(FArchive& Ar)
{
	// Serialize the usual UPROPERTIES
	Super::Serialize(Ar);

	if (GenerationParameters.GenerationStrategy == ESVOGenerationStrategy::UseBaked)
	{
		Ar << NavigationData.OctreeData;
	}
}

void ASVONVolume::BeginPlay()
{
	if (!bIsReadyForNavigation && GenerationParameters.GenerationStrategy == ESVOGenerationStrategy::GenerateOnBeginPlay)
	{
		Generate();
	}
	else
	{
		UpdateBounds();
	}

	bIsReadyForNavigation = true;

	if (USVONSubsystem* SvonSubsystem = GetWorld()->GetSubsystem<USVONSubsystem>())
	{
		SvonSubsystem->RegisterVolume(this);
	}
}

void ASVONVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (USVONSubsystem* SvonSubsystem = GetWorld()->GetSubsystem<USVONSubsystem>())
	{
		SvonSubsystem->UnRegisterVolume(this);
	}

	Super::EndPlay(EndPlayReason);
}

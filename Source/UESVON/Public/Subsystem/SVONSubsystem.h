#pragma once

#include <UESVON/Public/Interface/SVONSubsystemInterface.h>
#include <UESVON/Public/Interface/SVONCollisionQueryInterface.h>

#include <Subsystems/WorldSubsystem.h>

#include "SVONSubsystem.generated.h"

class ASVONVolume;
class USVONNavigationComponent;

UCLASS()
class UESVON_API USVONSubsystem : public UTickableWorldSubsystem, public ISVONSubsystemInterface, public ISVONCollisionQueryInterface
{
	GENERATED_BODY()

public:
	/* ISVONSubsystemInterface BEGIN */
	UFUNCTION()
	void RegisterVolume(const ASVONVolume* Volume) override;
	UFUNCTION()
	void UnRegisterVolume(const ASVONVolume* Volume) override;
	UFUNCTION()
	void RegisterNavComponent(USVONNavigationComponent* NavComponent) override;
	UFUNCTION()
	void UnRegisterNavComponent(USVONNavigationComponent* NavComponent) override;
	UFUNCTION()
	const ASVONVolume* GetVolumeForPosition(const FVector& Position) override;
	/* ISVONSubsystemInterface END */

	/* ISVONCollisionQueryInterface BEGIN */
	virtual bool IsBlocked(const FVector& Position, const float VoxelSize, ECollisionChannel CollisionChannel, const float AgentRadius) const override;
	/* ISVONCollisionQueryInterface END */

	virtual void Tick(float DeltaTime) override;
	TStatId GetStatId() const override;

private:
	UPROPERTY()
	TArray<const ASVONVolume*> RegisteredVolumes{};

	UPROPERTY()
	TArray<USVONNavigationComponent*> RegisteredNavComponents{};
};

#pragma once

#include <Subsystems/WorldSubsystem.h>

#include "SVONSubsystem.generated.h"

class ASVONVolume;
class USVONNavigationComponent;

UCLASS()
class UESVON_API USVONSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterVolume(const ASVONVolume* Volume);
	void UnRegisterVolume(const ASVONVolume* Volume);

	void RegisterNavComponent(USVONNavigationComponent* NavComponent);
	void UnRegisterNavComponent(USVONNavigationComponent* NavComponent);

	const ASVONVolume* GetVolumeForPosition(const FVector& Position);

	virtual void Tick(float DeltaTime) override;
	TStatId GetStatId() const override;

private:
	UPROPERTY()
	TArray<const ASVONVolume*> RegisteredVolumes{};

	UPROPERTY()
	TArray<USVONNavigationComponent*> RegisteredNavComponents{};
};

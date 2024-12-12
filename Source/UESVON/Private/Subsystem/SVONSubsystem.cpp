#include <UESVON/Public/Subsystem/SVONSubsystem.h>
#include <UESVON/Public/SVONVolume.h>
#include <UESVON/Public/SVONNavigationComponent.h>

void USVONSubsystem::RegisterVolume(const ASVONVolume* Volume)
{
	RegisteredVolumes.AddUnique(Volume);
}

void USVONSubsystem::UnRegisterVolume(const ASVONVolume* Volume)
{
	RegisteredVolumes.RemoveSingleSwap(Volume, EAllowShrinking::No);
}

void USVONSubsystem::RegisterNavComponent(USVONNavigationComponent* NavComponent)
{
	RegisteredNavComponents.AddUnique(NavComponent);
}

void USVONSubsystem::UnRegisterNavComponent(USVONNavigationComponent* NavComponent)
{
	RegisteredNavComponents.RemoveSingleSwap(NavComponent, EAllowShrinking::No);
}

const ASVONVolume* USVONSubsystem::GetVolumeForPosition(const FVector& Position)
{
	for (const ASVONVolume* Volume : RegisteredVolumes)
	{
		if (Volume->EncompassesPoint(Position)) 
		{
			return Volume;
		}
	}

	return nullptr;
}

void USVONSubsystem::Tick(float DeltaTime)
{
	for (USVONNavigationComponent* NavComponent : RegisteredNavComponents)
	{
		bool bIsInValidVolume = false;

		for (const ASVONVolume* Volume : RegisteredVolumes)
		{
			//float DistanceToPoint = 0.f;
			if (Volume->EncompassesPoint(NavComponent->GetPawnPosition()))//, 0.5f));//, &DistanceToPoint))
			{
				NavComponent->SetCurrentNavVolume(Volume);
				bIsInValidVolume = true;
				break;
			}
		}

		if (!bIsInValidVolume)
		{
			NavComponent->SetCurrentNavVolume(nullptr);
		}
	}
}

TStatId USVONSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USVONSubsystem, STATGROUP_Tickables);
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SVONNavigationComponent.generated.h"

class ASVONVolume;
struct SVONLink;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UESVON_API USVONNavigationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USVONNavigationComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;


	ASVONVolume* myCurrentNavVolume;
	bool HasNavVolume();

	bool FindVolume();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	SVONLink GetNavPosition(FVector& aPosition);

	void DebugLocalPosition(FVector& aPosition);


	
};

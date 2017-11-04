// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "TankTrack.generated.h"

/**
 * Used for driving force.
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class UWOT_API UTankTrack : public UStaticMeshComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float MovingForce = 10000000;
	
public:
	UFUNCTION(BlueprintCallable)
		void SetThrottle(float throttle);
};

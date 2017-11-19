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

private:
	float Throttle = 0;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float MovingForce = 10000000;

protected:
	void OnRegister() override;
	void TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction) override;

public:
	UFUNCTION(BlueprintCallable)
		void SetThrottle(const float throttle);
};

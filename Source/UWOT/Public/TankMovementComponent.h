// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/NavMovementComponent.h"
#include "TankMovementComponent.generated.h"

/**
 * Tank driving handler
 */
class UTankTrack;

UCLASS(ClassGroup = (UWOT), meta = (BlueprintSpawnableComponent))
class UWOT_API UTankMovementComponent : public UNavMovementComponent
{
	GENERATED_BODY()

public:
	UTankTrack* LeftTrack = nullptr;
	UTankTrack* RightTrack = nullptr;
	
private:
	void RequestDirectMove(const FVector& moveVelocity, bool bForceMaxSpeed) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Setup")
		void Init(UTankTrack * leftTrack, UTankTrack * rightTrack);
	UFUNCTION(BlueprintCallable, Category = "Input")
		void MoveBody(float throttleUnit);
	UFUNCTION(BlueprintCallable, Category = "Input")
		void RotateBody(float throttleUnit);
};

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

protected:
	UTankTrack* LeftTrack = nullptr;
	UTankTrack* RightTrack = nullptr;

public:
	UPROPERTY(BlueprintReadWrite)
		FVector2D MoveDirection;

private:
	void RequestDirectMove(const FVector& moveVelocity, bool bForceMaxSpeed) override;

protected:
	void BeginPlay() override;
	void TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Setup")
		void Init(UTankTrack * leftTrackInstance, UTankTrack * rightTrackInstace);
};

// Fill out your copyright notice in the Description page of Project Settings.

#include "TankMovementComponent.h"
#include "TankTrack.h"


void UTankMovementComponent::RequestDirectMove(const FVector& moveVelocity, bool bForceMaxSpeed)
{
	auto moveDirection = moveVelocity.GetSafeNormal();

	auto moveBodyDirectionThrottle = FVector::DotProduct(GetOwner()->GetActorForwardVector(), moveDirection);
	if (moveBodyDirectionThrottle > 0) MoveBody(1);
	else if (moveBodyDirectionThrottle < 0) MoveBody(-1);

	auto rotateBodyThrottle = FVector::DotProduct(GetOwner()->GetActorRightVector(), moveDirection);
	if (rotateBodyThrottle > 0) RotateBody(1);
	else if (rotateBodyThrottle < 0) RotateBody(-1);
	
}

void UTankMovementComponent::Init (UTankTrack * leftTrack, UTankTrack * rightTrack)
{
	if(!leftTrack || !rightTrack) return;
	LeftTrack = leftTrack;
	RightTrack = rightTrack;
}


void UTankMovementComponent::MoveBody (const float throttleUnit)
{
	LeftTrack->SetThrottle(throttleUnit);
	RightTrack->SetThrottle(throttleUnit);
}

void UTankMovementComponent::RotateBody (const float throttleUnit)
{
	LeftTrack->SetThrottle(throttleUnit);
	RightTrack->SetThrottle(-throttleUnit);
}


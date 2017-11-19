// Fill out your copyright notice in the Description page of Project Settings.

#include "TankMovementComponent.h"
#include "TankTrack.h"

void UTankMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	SetTickGroup(ETickingGroup::TG_StartPhysics);
}

void UTankMovementComponent::TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction * thisTickFunction)
{
	Super::TickComponent(deltaTime, tickType, thisTickFunction);

	// Set track's throttle based on moving direction
	if(!MoveDirection.IsZero())
	{
		MoveDirection.Normalize();
		if (MoveDirection.X > 0)
		{
			if (MoveDirection.Y > 0)
			{
				LeftTrack->SetThrottle(1);
				RightTrack->SetThrottle(1 - MoveDirection.Y * 2);
			}
			else
			{
				LeftTrack->SetThrottle(1 + MoveDirection.Y * 2);
				RightTrack->SetThrottle(1);
			}
		}
		else
		{
			if (MoveDirection.Y > 0)
			{
				LeftTrack->SetThrottle(-1);
				RightTrack->SetThrottle(MoveDirection.Y * 2 - 1);
			}
			else
			{
				LeftTrack->SetThrottle(-MoveDirection.Y * 2 - 1);
				RightTrack->SetThrottle(-1);
			}
		}
		MoveDirection = FVector2D::ZeroVector;
	}
}

void UTankMovementComponent::RequestDirectMove(const FVector& moveVelocity, bool bForceMaxSpeed)
{
	MoveDirection = FVector2D(FVector::DotProduct(GetOwner()->GetActorForwardVector(), moveVelocity), FVector::DotProduct(GetOwner()->GetActorRightVector(), moveVelocity));
}

void UTankMovementComponent::Init (UTankTrack * leftTrack, UTankTrack * rightTrack)
{
	if(!leftTrack || !rightTrack) return;

	LeftTrack = leftTrack;
	RightTrack = rightTrack;

	leftTrack->AddTickPrerequisiteComponent(this);
	rightTrack->AddTickPrerequisiteComponent(this);
}


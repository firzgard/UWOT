// Fill out your copyright notice in the Description page of Project Settings.

#include "TankTrack.h"

UTankTrack::UTankTrack()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetTickGroup(ETickingGroup::TG_StartPhysics);
}


void UTankTrack::TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction)
{
	Super::TickComponent(deltaTime, tickType, thisTickFunction);
	
	// Move the tank based on throttle and then reset it
	if(Throttle != 0)
	{
		const auto appliedForce = GetForwardVector() * Throttle * MovingForce;
		AddForceAtLocation(appliedForce, GetComponentLocation());
		Throttle = 0;
	}
}

void UTankTrack::SetThrottle(const float throttle)
{
	Throttle = FMath::Clamp<float>(throttle, -1, 1);
	
}

// Fill out your copyright notice in the Description page of Project Settings.

#include "TankTrack.h"

void UTankTrack::SetThrottle(float throttle)
{
	auto clampedThrottle = FMath::Clamp<int>(throttle, -1, 1);
	auto appliedForce = GetForwardVector() * clampedThrottle * MovingForce;

	Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent())->AddForceAtLocation(appliedForce, GetComponentLocation());
}
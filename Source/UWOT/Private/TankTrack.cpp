// Fill out your copyright notice in the Description page of Project Settings.

#include "TankTrack.h"


void UTankTrack::OnRegister()
{
	Super::OnRegister();
	PrimaryComponentTick.bCanEverTick = true;
	UE_LOG(LogTemp, Warning, TEXT("%f"), PrimaryComponentTick.bCanEverTick ? "true" : "false");
}

void UTankTrack::TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction)
{
	Super::TickComponent(deltaTime, tickType, thisTickFunction);
	UE_LOG(LogTemp, Warning, TEXT("%f"), Throttle);

	// Move the tank based on the setted throttle and the reset it
	if(Throttle != 0)
	{
		const auto appliedForce = GetForwardVector() * Throttle * MovingForce;
		Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent())->AddForceAtLocation(appliedForce, GetComponentLocation());
		Throttle = 0;
	}
}

void UTankTrack::SetThrottle(const float throttle)
{
	Throttle = FMath::Clamp<float>(throttle, -1, 1);
	
}

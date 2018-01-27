// Fill out your copyright notice in the Description page of Project Settings.

#include "TankPlayerController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStaticsTypes.h"

#include "TankMainWeaponComponent.h"

void ATankPlayerController::BeginPlay()
{
	Super::BeginPlay();

	PrimaryActorTick.bCanEverTick = true;
	SetTickGroup(ETickingGroup::TG_PrePhysics);
}

void ATankPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	const auto tank = Cast<ATank>(InPawn);
	const auto lastTank = ControlledTank;
	ControlledTank = tank;
	ReceiveSetTank(lastTank);
}

void ATankPlayerController::Tick(float deltaTime)
{
	Super::Tick(deltaTime);


}


void ATankPlayerController::GetAimingTargetPosition(FVector const &CursorWorldLocation, FVector const &CursorWorldDirection, float const LineTraceRange, FVector &OutTargetPosition) const
{
	// Safe-guard in case the cursor's camera clip through a wall behind or something.
	float const LINE_TRACE_START_DISTANCE_FROM_CURSOR = 500;

	FHitResult OutHitresult;
	auto lineTraceStartPos = CursorWorldLocation + CursorWorldDirection * LINE_TRACE_START_DISTANCE_FROM_CURSOR;
	auto lineTraceEndPos = lineTraceStartPos + CursorWorldDirection * LineTraceRange;
	
	if (GetWorld()->LineTraceSingleByChannel(OutHitresult, lineTraceStartPos, lineTraceEndPos, ECollisionChannel::ECC_Camera))
	{
		OutTargetPosition = OutHitresult.Location;
		
		// If hit a tank, highlight it
		if(OutHitresult.Actor.IsValid())
		{
			if (auto hitTank = Cast<ATank>(OutHitresult.Actor.Get()))
			{
				hitTank->SetHighlight(true);
			}
		}
	}
	else
	{
		OutTargetPosition = lineTraceEndPos;
	}
}


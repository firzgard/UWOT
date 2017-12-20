// Fill out your copyright notice in the Description page of Project Settings.

#include "TankPlayerController.h"
#include "TimerManager.h"
#include "Engine/World.h"


void ATankPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetTickGroup(ETickingGroup::TG_PrePhysics);
}

void ATankPlayerController::GetAimingTargetPosition(FVector const &CursorWorldLocation, FVector const &CursorWorldDirection, float const LineTraceRange, FVector &OutTargetPosition) const
{
	// Safe-guard in case the cursor's camera clip through a wall behind or something.
	float const LINE_TRACE_START_DISTANCE_FROM_CURSOR = 500;

	FHitResult OutHitresult;
	auto lineTraceStartPos = CursorWorldLocation + CursorWorldDirection * LINE_TRACE_START_DISTANCE_FROM_CURSOR;
	auto lineTraceEndPos = lineTraceStartPos + CursorWorldDirection * LineTraceRange;

	if (GetWorld()->LineTraceSingleByChannel(OutHitresult, lineTraceStartPos, lineTraceEndPos, ECollisionChannel::ECC_Visibility))
	{
		OutTargetPosition = OutHitresult.Location;
	}
	else
	{
		OutTargetPosition = lineTraceEndPos;
	}
}

ATank * ATankPlayerController::GetControlledTank()
{
	if(!ControlledTank)
	{
		ControlledTank = Cast<ATank>(GetPawn());
	}
	return ControlledTank;
}


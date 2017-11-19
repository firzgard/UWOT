// Fill out your copyright notice in the Description page of Project Settings.

#include "TankPlayerController.h"


void ATankPlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetTickGroup(ETickingGroup::TG_PrePhysics);
}

void ATankPlayerController::GetAimingTargetPosition(FVector const &CursorWorldLocation, FVector const &CursorWorldDirection, float const LineTraceRange, FVector &OutTargetPosition) const
{
	FHitResult OutHitresult;
	auto lineTraceEndPos = CursorWorldLocation + CursorWorldDirection * LineTraceRange;

	if (GetWorld()->LineTraceSingleByChannel(OutHitresult, CursorWorldLocation, lineTraceEndPos, ECollisionChannel::ECC_Visibility))
	{
		OutTargetPosition = OutHitresult.Location;
	}
	else
	{
		OutTargetPosition = lineTraceEndPos;
	}
}


#pragma region PUBLIC


#pragma endregion PUBLIC


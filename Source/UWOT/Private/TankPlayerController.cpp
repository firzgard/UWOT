// Fill out your copyright notice in the Description page of Project Settings.

#include "TankPlayerController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStaticsTypes.h"

#include "TankMainWeaponComponent.h"
#include "TankSpottingComponent.h"

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

	if (tank != ControlledTank)
	{
		// Reset last possessed tank
		if (ControlledTank)
		{
			ControlledTank->SpottingComponent->TeamId = ETankTeamEnum::NONE;
		}

		ControlledTank = tank;

		if (ControlledTank)
		{
			ControlledTank->SpottingComponent->TeamId = TeamId;
		}
	}

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

	FHitResult outHitresult;
	const auto lineTraceStartPos = CursorWorldLocation + CursorWorldDirection * LINE_TRACE_START_DISTANCE_FROM_CURSOR;
	const auto lineTraceEndPos = lineTraceStartPos + CursorWorldDirection * LineTraceRange;

	auto collisionQueryParams = FCollisionQueryParams();
	if(ControlledTank)
	{
		collisionQueryParams.AddIgnoredActor(ControlledTank);
	}
	
	if (GetWorld()->LineTraceSingleByChannel(outHitresult, lineTraceStartPos, lineTraceEndPos, ECollisionChannel::ECC_Camera, collisionQueryParams))
	{
		OutTargetPosition = outHitresult.Location;
		
		// If hit a tank, highlight it
		if(outHitresult.Actor.IsValid())
		{
			if (auto hitTank = Cast<ATank>(outHitresult.Actor.Get()))
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

bool ATankPlayerController::OnSpottedSelf_Implementation(bool bSpotted)
{
	ReceiveOnSpottedSeft(bSpotted);
	return true;
}

bool ATankPlayerController::OnSpottedOther_Implementation(bool bSpotted, ATank * other)
{
	return true;
}

ETankTeamEnum ATankPlayerController::GetTeamId()
{
	return TeamId;
}

ATank* ATankPlayerController::GetControlledTank()
{
	return ControlledTank;
}


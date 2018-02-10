// Fill out your copyright notice in the Description page of Project Settings.

#include "TankAIController.h"

#include "Tank.h"
#include "TankMainWeaponComponent.h"
#include "TankMovementComponent.h"
#include "TankSpottingComponent.h"

#include "Engine/World.h"
#include "Kismet/GameplayStaticsTypes.h"

void ATankAIController::BeginPlay()
{
	Super::BeginPlay();

	SetTickGroup(ETickingGroup::TG_PrePhysics);

	ControlledTank->SetActorHiddenInGame(true);
}

void ATankAIController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	const auto tank = Cast<ATank>(InPawn);

	if(tank != ControlledTank)
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
}

void ATankAIController::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	if(TargetTank && ControlledTank)
	{
		auto outHitResult = FPredictProjectilePathResult();
		ControlledTank->MainWeaponComponent->TraceProjectilePath(outHitResult);
		const auto bTargetLocked = outHitResult.HitResult.GetActor() == TargetTank;

		ControlledTank->MainWeaponComponent->AimGun(TargetTank->GetAiTargetLocation(), bDrawAimingDebugLine);

		auto const towardPlayerVector = TargetTank->GetActorLocation() - ControlledTank->GetActorLocation();
		auto const bIsAhead = (towardPlayerVector | ControlledTank->GetActorForwardVector()) > 0;

		// Move toward player if distance-to-player is bigger than AcceptanceDistance or cannot lock gun into player
		// and is in front of controlled tank
		if (bIsAhead && (towardPlayerVector.SizeSquared() > AcceptanceDistance * AcceptanceDistance || !bTargetLocked))
		{
			// Try to move using navmesh if possible
			MoveToActor(TargetTank, AcceptanceDistance, true, true);
		}

		// Else rotate the body to angle against player attack
		// or rotate to face player before moving
		else
		{
			// Rotate toward the best angled position
			// There will be 2 such position, one on the left and other on the right
			// Choose the position that player is facing toward, in advance of them moving forward
			const auto bestAngle = (TargetTank->GetActorForwardVector() | ControlledTank->GetActorRightVector()) > 0 ? BestAngleDeg : -BestAngleDeg;

			const auto bestAngleDirection = towardPlayerVector.RotateAngleAxis(bestAngle, ControlledTank->GetActorUpVector()).GetSafeNormal();
			const auto rotateRightThrust = bestAngleDirection | ControlledTank->GetActorRightVector();

			if (FMath::Abs(rotateRightThrust) > FMath::Sin(FMath::DegreesToRadians(BestAngleToleranceDeg)))
			{
				ControlledTank->MovementComponent->SetTargetGear(1, true);
				ControlledTank->MovementComponent->SetThrottleInput(1);

				ControlledTank->MovementComponent->SetLeftThrustInput(rotateRightThrust);
				ControlledTank->MovementComponent->SetRightThrustInput(-rotateRightThrust);
			}
		}

		// Firing action
		if (bFirable)
		{
			// Fire if tracing hits player
			if (bTargetLocked)
			{
				ControlledTank->TryFireGun();
			}
		}
	}
}

bool ATankAIController::OnSpottedSelf_Implementation(bool bSpotted)
{
	ControlledTank->SetActorHiddenInGame(!bSpotted);

	return true;
}

bool ATankAIController::OnSpottedOther_Implementation(bool bSpotted, ATank * other)
{
	if(bSpotted)
	{
		TargetTank = other;
	}
	else if (TargetTank == other)
	{
		TargetTank = nullptr;
	}
	

	return true;
}

ETankTeamEnum ATankAIController::GetTeamId()
{
	return TeamId;
}

ATank* ATankAIController::GetControlledTank()
{
	return ControlledTank;
}

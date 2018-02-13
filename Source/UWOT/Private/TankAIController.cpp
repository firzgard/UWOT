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
		ControlledTank = tank;

		if (IsValid(ControlledTank))
		{
			ControlledTank->SpottingComponent->TeamId = TeamId;
		}
	}
}

void ATankAIController::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	if(bHasTarget && ControlledTank)
	{
		auto bTargetLocked = false;

		if(IsValid(TargetTank))
		{
			auto outHitResult = FPredictProjectilePathResult();
			ControlledTank->MainWeaponComponent->TraceProjectilePath(outHitResult);

			bTargetLocked = outHitResult.HitResult.GetActor() == TargetTank;

			LastSpottedTargetLocation = TargetTank->GetAiTargetLocation();
		}

		auto const towardPlayerVector = LastSpottedTargetLocation - ControlledTank->GetActorLocation();
		auto const bIsInsideAcceptanceRadius = towardPlayerVector.SizeSquared() < AcceptanceDistance * AcceptanceDistance;

		// Move toward player if distance-to-player is bigger than AcceptanceDistance or cannot lock gun into player
		if (!bIsInsideAcceptanceRadius || !bTargetLocked)
		{
			// Try to move using navmesh if possible
			MoveToLocation(LastSpottedTargetLocation, AcceptanceDistance, true, true);
		}

		// Else rotate the body to angle against player attack
		// or rotate to face player before moving
		else
		{
			StopMovement();

			// Rotate toward the best angled position
			// There will be 2 such position, one on the left and other on the right
			// Choose the position that player is facing toward, in advance of them moving forward
			const auto bestAngle = (LastSpottedTargetLocation | ControlledTank->GetActorRightVector()) > 0 ? BestAngleDeg : -BestAngleDeg;

			const auto bestAngleDirection = towardPlayerVector.RotateAngleAxis(bestAngle, ControlledTank->GetActorUpVector()).GetSafeNormal();
			auto rotateRightThrust = bestAngleDirection | ControlledTank->GetActorRightVector();

			if (FMath::Abs(rotateRightThrust) > FMath::Sin(FMath::DegreesToRadians(BestAngleToleranceDeg)))
			{
				// Normalize thrust
				rotateRightThrust /= FMath::Abs(rotateRightThrust);

				ControlledTank->MovementComponent->SetTargetGear(1, true);
				ControlledTank->MovementComponent->SetThrottleInput(1);
				ControlledTank->MovementComponent->SetLeftThrustInput(rotateRightThrust);
				ControlledTank->MovementComponent->SetRightThrustInput(-rotateRightThrust);
			}
		}

		// Reset last spotted target location if
		// Already lost spotting on target tank
		// but still have not reached last target
		if(!IsValid(TargetTank) && bHasTarget && bIsInsideAcceptanceRadius)
		{
			bHasTarget = false;
		}


		ControlledTank->MainWeaponComponent->AimGun(LastSpottedTargetLocation, bDrawAimingDebugLine);

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

float ATankAIController::TakeDamage(float damage, FDamageEvent const& damageEvent, AController* eventInstigator,
	AActor* damageCauser)
{
	// Respond to getting hitted if not already has target
	if(!TargetTank)
	{
		bHasTarget = true;
		LastSpottedTargetLocation = damageCauser->GetActorLocation();
	}
	
	return Super::TakeDamage(damage, damageEvent, eventInstigator, damageCauser);
}

void ATankAIController::OnSpottedSelf_Implementation(bool bSpotted)
{
	ControlledTank->SetActorHiddenInGame(!bSpotted);
}

void ATankAIController::OnSpottedOther_Implementation(bool bSpotted, ATank * other)
{
	if(bSpotted)
	{
		if (!IsValid(TargetTank))
		{
			TargetTank = other;
			bHasTarget = true;
		}
	}

	else if (TargetTank == other)
	{
		TargetTank = nullptr;
	}
}

ETankTeamEnum ATankAIController::GetTeamId()
{
	return TeamId;
}

ATank* ATankAIController::GetControlledTank()
{
	return ControlledTank;
}

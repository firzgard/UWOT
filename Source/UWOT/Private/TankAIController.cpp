// Fill out your copyright notice in the Description page of Project Settings.

#include "TankAIController.h"

#include "Tank.h"
#include "TankMainWeaponComponent.h"
#include "TankMovementComponent.h"

#include "Engine/World.h"
#include "Kismet/GameplayStaticsTypes.h"

void ATankAIController::BeginPlay()
{
	Super::BeginPlay();

	SetTickGroup(ETickingGroup::TG_PrePhysics);
}

void ATankAIController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	const auto tank = Cast<ATank>(InPawn);
	if (tank)
	{
		ControlledTank = tank;
	}
}

void ATankAIController::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	auto player = GetWorld()->GetFirstPlayerController()->GetPawn();

	if (player && ControlledTank)
	{
		ControlledTank->MainWeaponComponent->AimGun(player->GetActorLocation(), bDrawAimingDebugLine);

		auto const towardPlayerVector = player->GetActorLocation() - ControlledTank->GetActorLocation();
		auto const bIsAhead = (towardPlayerVector | ControlledTank->GetActorForwardVector()) > 0;

		// Move toward player if distance-to-player is bigger than AcceptanceDistance and is in front of controlled tank
		if (towardPlayerVector.SizeSquared() > AcceptanceDistance * AcceptanceDistance && bIsAhead)
		{
			MoveToActor(player, AcceptanceDistance, true, false);
		}
		else // Else angle the body to defend against player attack
		{
			// Rotate toward the best angled position
			// There will be 2 such position, one on the left and other on the right
			// Choose the position that player is facing toward, in advance of them moving forward
			const auto bestAngle = (player->GetActorForwardVector() | ControlledTank->GetActorRightVector()) > 0 ? BestAngleDeg : -BestAngleDeg;

			const auto bestAngleDirection = towardPlayerVector.RotateAngleAxis(bestAngle, ControlledTank->GetActorUpVector()).GetSafeNormal();
			const auto rotateRightThrust = bestAngleDirection | ControlledTank->GetActorRightVector();

			if(FMath::Abs(rotateRightThrust) > FMath::Sin(FMath::DegreesToRadians(BestAngleToleranceDeg)))
			{
				ControlledTank->MovementComponent->SetTargetGear(1, false);
				ControlledTank->MovementComponent->SetThrottleInput(1);

				ControlledTank->MovementComponent->SetLeftThrustInput(rotateRightThrust);
				ControlledTank->MovementComponent->SetRightThrustInput(-rotateRightThrust);
			}
		}
		
		// Firing action
		if (bFirable)
		{
			// Fire if tracing hits player
			auto outHitResult = FPredictProjectilePathResult();
			ControlledTank->MainWeaponComponent->TraceProjectilePath(outHitResult);
			if(outHitResult.HitResult.GetActor() == player)
			{
				ControlledTank->MainWeaponComponent->TryFireGun();
			}
		}
	}
}
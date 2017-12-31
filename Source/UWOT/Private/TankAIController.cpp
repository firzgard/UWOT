// Fill out your copyright notice in the Description page of Project Settings.

#include "TankAIController.h"

#include "Engine/World.h"
#include "Kismet/GameplayStaticsTypes.h"

#include "Tank.h"
#include "TankMainWeaponComponent.h"


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

		auto towardPlayerVector = player->GetActorLocation() - ControlledTank->GetActorLocation();

		// Move toward player if distance-to-player is bigger than AcceptanceDistance
		if (towardPlayerVector.SizeSquared() > AcceptanceDistance * AcceptanceDistance)
		{
			MoveToActor(player, AcceptanceDistance, true, false);
		}
		else // Else angle the body to defend against player attack
		{
			// Rotate toward the best angled position
			// There will be 2 such position, one on the left and other on the right
			// Choose the position that player is facing toward, in advance of them moving forward
			auto bestAngle = (player->GetActorForwardVector() | GetActorRightVector()) > 0 ? BestAngleDeg : -BestAngleDeg;

			auto bestAngleDirection = towardPlayerVector.RotateAngleAxis(bestAngle, GetActorUpVector()).GetSafeNormal();
			auto rotateRightThrottle = bestAngleDirection | GetActorRightVector();

			if(FMath::Abs(rotateRightThrottle) > FMath::Sin(FMath::DegreesToRadians(BestAngleToleranceDeg)))
			{
				if (rotateRightThrottle > 0)
				{
					ControlledTank->RotateBody(1);
				}
				else if (rotateRightThrottle < 0)
				{
					ControlledTank->RotateBody(-1);
				}
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
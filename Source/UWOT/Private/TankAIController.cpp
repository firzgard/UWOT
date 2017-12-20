// Fill out your copyright notice in the Description page of Project Settings.

#include "TankAIController.h"
#include "Engine/World.h"
#include "Tank.h"
#include "TankMainWeaponComponent.h"

void ATankAIController::BeginPlay()
{
	Super::BeginPlay();

	SetTickGroup(ETickingGroup::TG_PrePhysics);
}

void ATankAIController::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	auto player = GetWorld()->GetFirstPlayerController()->GetPawn();

	if (player && GetControlledTank())
	{
		ControlledTank->MainWeaponComponent->AimGun(player->GetActorLocation(), AimingTraceOption, bDrawAimingDebugLine);

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
			auto bestAngle = FVector::DotProduct(player->GetActorForwardVector(), GetActorRightVector()) > 0 ? BestAngleDeg : -BestAngleDeg;

			auto bestAngleDirection = towardPlayerVector.RotateAngleAxis(bestAngle, GetActorUpVector()).GetSafeNormal();
			auto rotateRightThrottle = FVector::DotProduct(bestAngleDirection, GetActorRightVector());

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
		auto outResult = FPredictProjectilePathResult();
		

		// Firing action
		if (bFirable && ControlledTank->MainWeaponComponent->CheckIsTargetInAim(player))
		{
			ControlledTank->MainWeaponComponent->TryFireGun();
		}
	}
}



ATank * ATankAIController::GetControlledTank()
{
	if (!ControlledTank)
	{
		ControlledTank = Cast<ATank>(GetPawn());
	}
	return ControlledTank;
}
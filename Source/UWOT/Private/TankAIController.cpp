// Fill out your copyright notice in the Description page of Project Settings.

#include "TankAIController.h"
#include "Engine/World.h"
#include "Tank.h"
#include "TankMainWeaponComponent.h"

void ATankAIController::BeginPlay()
{
	Super::BeginPlay();
	SetTickGroup(ETickingGroup::TG_PrePhysics);

	ControlledTank = Cast<ATank>(GetPawn());
}

void ATankAIController::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	auto player = GetWorld()->GetFirstPlayerController()->GetPawn();

	if (player)
	{
		ControlledTank->MainWeaponComponent->AimGun(player->GetActorLocation(), AimingTraceOption, bDrawAimingDebugLine);

		auto towardPlayerVector = player->GetActorLocation() - ControlledTank->GetActorLocation();
		auto distanceToPlayerSqr = towardPlayerVector.SizeSquared();

		// Move toward player if distance-to-player is bigger than AcceptanceDistance
		if (distanceToPlayerSqr > AcceptanceDistance * AcceptanceDistance)
		{
			MoveToActor(player, AcceptanceDistance, true, false);
		}
		else // Else angle the body to defend against player attack
		{
			auto rotateBodyThrottle = FVector::DotProduct(ControlledTank->GetActorRightVector(), towardPlayerVector);

			if (rotateBodyThrottle > 0) ControlledTank->RotateBody(1);
			else if (rotateBodyThrottle < 0) ControlledTank->RotateBody(-1);
		}
	}
}

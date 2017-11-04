// Fill out your copyright notice in the Description page of Project Settings.

#include "TankAIController.h"
#include "Engine/World.h"
#include "Tank.h"

void ATankAIController::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	auto player = GetWorld()->GetFirstPlayerController()->GetPawn();
	auto tank = Cast<ATank>(GetPawn());

	if (player)
	{
		tank->AimGun(player->GetActorLocation(), AimingTraceOption, bDrawAimingDebugLine);

		auto towardPlayerVector = player->GetActorLocation() - tank->GetActorLocation();
		auto distanceToPlayerSqr = towardPlayerVector.SizeSquared();

		// Move toward player if distance-to-player is bigger than AcceptanceDistance
		if (distanceToPlayerSqr > AcceptanceDistance * AcceptanceDistance)
		{
			MoveToActor(player, AcceptanceDistance, true, false);
		}
		else // Else angle the body to defend against player attack
		{
			auto rotateBodyThrottle = FVector::DotProduct(tank->GetActorRightVector(), towardPlayerVector);

			if (rotateBodyThrottle > 0) tank->RotateBody(1);
			else if (rotateBodyThrottle < 0) tank->RotateBody(-1);
		}
		
	}
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Tank.h"

#include "CoreMinimal.h"
#include "AIController.h"
#include "TankAIController.generated.h"

/**
 * 
 */
UCLASS()
class UWOT_API ATankAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	// How far will the AI tank move toward player before stopping
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Properties")
		float AcceptanceDistance = 3000;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Logging Properties")
		TEnumAsByte<ESuggestProjVelocityTraceOption::Type> AimingTraceOption = ESuggestProjVelocityTraceOption::DoNotTrace;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Logging Properties")
		bool bDrawAimingDebugLine = false;

public:
	virtual void Tick(float deltaTime) override;
};

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

private: 
	ATank * ControlledTank;

public:
	/** How far will the AI tank move toward player before stopping */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|AI Properties")
		float AcceptanceDistance = 3000;
	/** At which angle should the AI tank face player to increase effective armor */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|AI Properties")
		float BestAngleDeg = 30;
	/** Largest angle in degree between best angle and real angle that is considered acceptable */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|AI Properties")
		float BestAngleToleranceDeg = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Logging Properties")
		bool bDrawAimingDebugLine = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|AI Properties")
		bool bFirable = true;

public:
	void BeginPlay() override;
	void Tick(float deltaTime) override;

	UFUNCTION(BlueprintCallable)
		ATank * GetControlledTank();
};

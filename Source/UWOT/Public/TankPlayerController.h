// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Tank.h"

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TankPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class UWOT_API ATankPlayerController : public APlayerController
{
	GENERATED_BODY()
	
private:

	UFUNCTION(BlueprintCallable)
	void GetAimingTargetPosition(FVector const &CursorWorldLocation, FVector const &CursorWorldDirection, float const LineTraceRange, FVector& OutTargetPosition) const;

public:
};

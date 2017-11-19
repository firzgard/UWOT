// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "Tank.generated.h"

class UTankAimingComponent;
class UTankMovementComponent;

UCLASS()
class UWOT_API ATank : public APawn
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
		UTankAimingComponent* AimingComponent = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
		UTankMovementComponent* MovementComponent = nullptr;

protected:
	void BeginPlay() override;
	void Tick(float deltaTime) override;

public:
	ATank();

	// Called to bind functionality to input
	void SetupPlayerInputComponent(class UInputComponent* playerInputComponent) override;

	void RotateBody(const float throttleUnit);

	UFUNCTION(BlueprintCallable, Category = "Input")
		void AimGun(const FVector & targetLocation
			, const ESuggestProjVelocityTraceOption::Type traceOption  = ESuggestProjVelocityTraceOption::DoNotTrace
			, const bool bDrawDebug  = false);
	UFUNCTION(BlueprintCallable, Category = "Input")
		void FireGun();
};

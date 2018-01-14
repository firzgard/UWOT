// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Tank.generated.h"

class UCamouflageComponent;
class UTankCameraMovementComponent;
class UTankMainWeaponComponent;
class UTankMovementComponent;

UCLASS()
class UWOT_API ATank : public APawn
{
	GENERATED_BODY()

protected:
	UPROPERTY()
		FVector2D DesiredMoveDirectionInput;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UTankCameraMovementComponent * CameraMovementComponent = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UTankMainWeaponComponent * MainWeaponComponent = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UTankMovementComponent * MovementComponent = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UCamouflageComponent * CamouflageComponent = nullptr;

protected:
	void BeginPlay() override;
	void Tick(float deltaTime) override;
	float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	void DisplayDebug(class UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;

public:
	ATank();

	// Called to bind functionality to input
	void SetupPlayerInputComponent(class UInputComponent* playerInputComponent) override;

	void RotateBody(const float throttleUnit);

	UFUNCTION(BlueprintCallable)
		float GetHullAlignment() const;
	UFUNCTION(BlueprintCallable)
		float GetTurretAlignment() const;
};

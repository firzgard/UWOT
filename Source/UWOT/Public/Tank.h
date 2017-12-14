// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "Tank.generated.h"

class UTankMainWeaponComponent;
class UTankMovementComponent;
class UCamouflageComponent;

UCLASS()
class UWOT_API ATank : public APawn
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
		UTankMainWeaponComponent* MainWeaponComponent = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
		UTankMovementComponent* MovementComponent = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
		UCamouflageComponent* CamouflageComponent = nullptr;

protected:
	void BeginPlay() override;
	void Tick(float deltaTime) override;

public:
	ATank();

	// Called to bind functionality to input
	void SetupPlayerInputComponent(class UInputComponent* playerInputComponent) override;

	void RotateBody(const float throttleUnit);
};

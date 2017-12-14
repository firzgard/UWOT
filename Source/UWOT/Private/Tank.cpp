// Fill out your copyright notice in the Description page of Project Settings.

#include "Tank.h"
#include "TankMainWeaponComponent.h"
#include "TankMovementComponent.h"
#include "CamouflageComponent.h"

// Sets default values
ATank::ATank()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MainWeaponComponent = CreateDefaultSubobject<UTankMainWeaponComponent>(FName("TankMainWeaponComponent"));
	MovementComponent = CreateDefaultSubobject<UTankMovementComponent>(FName("TankMovementComponent"));
	CamouflageComponent = CreateDefaultSubobject<UCamouflageComponent>(FName("TankCamouflageComponent"));
}

// Called when the game starts or when spawned
void ATank::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ATank::Tick(float deltaTime)
{
	Super::Tick(deltaTime);
}

// Called to bind functionality to input
void ATank::SetupPlayerInputComponent(UInputComponent* playerInputComponent)
{
	Super::SetupPlayerInputComponent(playerInputComponent);
}

void ATank::RotateBody(const float throttleUnit)
{
	MovementComponent->MoveDirection = FVector2D(0, throttleUnit);
}

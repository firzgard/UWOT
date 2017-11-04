// Fill out your copyright notice in the Description page of Project Settings.

#include "Tank.h"
#include "TankAimingComponent.h"
#include "TankMovementComponent.h"


// Sets default values
ATank::ATank()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AimingComponent = CreateDefaultSubobject<UTankAimingComponent>(FName("TankAimingComponent"));
	MovementComponent = CreateDefaultSubobject<UTankMovementComponent>(FName("TankMovementComponent"));
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

void ATank::RotateBody(float throttleUnit)
{
	MovementComponent->RotateBody(throttleUnit);
}

void ATank::AimGun(const FVector & targetLocation
	, const ESuggestProjVelocityTraceOption::Type traceOption
	, const bool bDrawDebug)
{
	AimingComponent->AimGun(targetLocation, traceOption, bDrawDebug);
}

void ATank::FireGun()
{
	AimingComponent->FireGun();
}

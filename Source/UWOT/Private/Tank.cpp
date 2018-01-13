// Fill out your copyright notice in the Description page of Project Settings.

#include "Tank.h"

#include "CamouflageComponent.h"
#include "TankCameraMovementComponent.h"
#include "TankMainWeaponComponent.h"
#include "TankMovementComponent.h"


// Sets default values
ATank::ATank()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraMovementComponent = CreateDefaultSubobject<UTankCameraMovementComponent>(FName("TankCameraMovementComponent"));
	MainWeaponComponent = CreateDefaultSubobject<UTankMainWeaponComponent>(FName("TankMainWeaponComponent"));
	MovementComponent = CreateDefaultSubobject<UTankMovementComponent>(FName("TankMovementComponent"));
	CamouflageComponent = CreateDefaultSubobject<UCamouflageComponent>(FName("TankCamouflageComponent"));
}

void ATank::BeginPlay()
{
	Super::BeginPlay();
}

void ATank::Tick(float deltaTime)
{
	Super::Tick(deltaTime);
}

float ATank::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	return Damage;
}

void ATank::SetupPlayerInputComponent(UInputComponent* playerInputComponent)
{
	Super::SetupPlayerInputComponent(playerInputComponent);
}

void ATank::RotateBody(const float throttleUnit)
{
//	MovementComponent->MoveDirection = FVector2D(0, throttleUnit);
}

float ATank::GetHullAlignment() const
{
	const auto hullYawVector = FVector::VectorPlaneProject(GetActorForwardVector(), CameraMovementComponent->GetCameraUpVector()).GetSafeNormal();
	auto angle = FMath::RadiansToDegrees(FMath::Acos(hullYawVector | CameraMovementComponent->GetCameraForwardVector()));
	return (hullYawVector | CameraMovementComponent->GetCameraRightVector()) > 0 ? angle : -angle;
}

float ATank::GetTurretAlignment() const
{
	const auto turretYawVector = FVector::VectorPlaneProject(MainWeaponComponent->GetTurretForwardVector(), CameraMovementComponent->GetCameraUpVector()).GetSafeNormal();
	auto angle = FMath::RadiansToDegrees(FMath::Acos(turretYawVector | CameraMovementComponent->GetCameraForwardVector()));
	return (turretYawVector | CameraMovementComponent->GetCameraRightVector()) > 0 ? angle : -angle;
}
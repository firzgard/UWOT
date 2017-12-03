// Fill out your copyright notice in the Description page of Project Settings.

#include "TankMainWeaponComponent.h"
#include "Projectile.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"


// Sets default values for this component's properties
UTankMainWeaponComponent::UTankMainWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}



void UTankMainWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UTankMainWeaponComponent::TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction)
{
	Super::TickComponent(deltaTime, tickType, thisTickFunction);
	
	if (RemainReloadTime > 0)
	{
		RemainReloadTime = FMath::Clamp<float>(RemainReloadTime - deltaTime, 0, ReloadTime);
		OnReloadGun.Broadcast(RemainReloadTime, ReloadTime);
	}
}


#pragma region PRIVATE

void UTankMainWeaponComponent::ElevateGun(const FVector & targetBarrelWorldDirection)
{
	// Find the delta between desired local pitch and current local pitch
	// by projecting the desired world direction to the local pitch plane.
	auto targetFiringLocalPitchDirection = targetBarrelWorldDirection - FVector::DotProduct(Barrel->GetRightVector(), targetBarrelWorldDirection) * Barrel->GetRightVector();
	targetFiringLocalPitchDirection.Normalize();
	const auto deltaPitch = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(targetFiringLocalPitchDirection, Barrel->GetForwardVector())));

	// Correct deltaPitch direction
	const auto correctedDeltaPitch = FVector::DotProduct(targetBarrelWorldDirection, Barrel->GetUpVector()) > 0
		? deltaPitch
		: -deltaPitch;

	// Clamp the delta pitch for this frame
	const auto deltaBarrelElevationSpeed = BarrelElevationSpeed * GetWorld()->DeltaTimeSeconds;
	const auto clampedDeltaPitch = FMath::Clamp<float>(correctedDeltaPitch, -deltaBarrelElevationSpeed, deltaBarrelElevationSpeed);

	// Clamp the local pitch to gun's current depression/elevation limit
	const auto currentGunDepressionLimit = GunDepressionAngleCurve->GetFloatValue(Turret->GetRelativeRotationCache().GetCachedRotator().Yaw);
	const auto clampedNewLocalPitch = FMath::Clamp<float>(Barrel->GetRelativeRotationCache().GetCachedRotator().Pitch + clampedDeltaPitch, currentGunDepressionLimit, GunElevationAngle);

	Barrel->SetRelativeRotation(FRotator(clampedNewLocalPitch, 0, 0));
}

void UTankMainWeaponComponent::RotateTurret(const FVector & targetTurretWorldDirection)
{
	// Find the delta between desired local yaw and current local yaw
	// by projecting the desired world direction to the local yaw plane.
	auto targetFiringLocalYawDirection = targetTurretWorldDirection - FVector::DotProduct(Turret->GetUpVector(), targetTurretWorldDirection) * Turret->GetUpVector();
	targetFiringLocalYawDirection.Normalize();
	const auto deltaYaw = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(targetFiringLocalYawDirection, Turret->GetForwardVector())));

	// Correct deltaYaw direction
	auto correctedDeltaYaw = FVector::DotProduct(targetTurretWorldDirection, Turret->GetRightVector()) > 0
		? deltaYaw
		: -deltaYaw;

	// Clamp the delta yaw for this frame
	const auto deltaTurretRotationSpeed = TurretRotationSpeed * GetWorld()->DeltaTimeSeconds;
	const auto clampedDeltaYaw = FMath::Clamp<float>(correctedDeltaYaw, -deltaTurretRotationSpeed, deltaTurretRotationSpeed);

	Turret->AddRelativeRotation(FRotator(0, clampedDeltaYaw, 0));
}

#pragma endregion PRIVATE


#pragma region PUBLIC

void UTankMainWeaponComponent::Init(UStaticMeshComponent * turret, UStaticMeshComponent * barrel, USceneComponent * firingPosition)
{
	if (!turret || !barrel || !firingPosition) return;
	Turret = turret;
	Barrel = barrel;
	FiringStartPosition = firingPosition;
}

void UTankMainWeaponComponent::AimGun(const FVector & targetLocation
	, const ESuggestProjVelocityTraceOption::Type traceOption
	, const bool bDrawDebug)
{
	RotateTurret(targetLocation - Turret->GetComponentLocation());

	FVector outVelocity;

	// Check aim solution
	// Use straight line between target and firing location if no solution
	if (!UGameplayStatics::SuggestProjectileVelocity(this, outVelocity, FiringStartPosition->GetComponentLocation(), targetLocation
		, ProjectileSpeed
		, false, 0, 0, traceOption
		, FCollisionResponseParams::DefaultResponseParam
		, TArray<AActor *>()
		, bDrawDebug))
	{
		outVelocity = targetLocation - FiringStartPosition->GetComponentLocation();
	}

	ElevateGun(outVelocity);
}

bool UTankMainWeaponComponent::TryFireGun()
{
	if (RemainReloadTime > 0) return false;
		
	GetWorld()->SpawnActor<AProjectile>(Projectile, FiringStartPosition->GetComponentTransform());
	RemainReloadTime = ReloadTime;
	return true;
}

void UTankMainWeaponComponent::ChangeShellType(TSubclassOf<AProjectile> newShellType)
{
	Projectile = newShellType;
	RemainReloadTime = ReloadTime = newShellType->GetDefaultObject<AProjectile>()->GetReloadTime();
	ProjectileSpeed = newShellType->GetDefaultObject<AProjectile>()->GetSpeed();
}

#pragma endregion PUBLIC



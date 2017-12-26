// Fill out your copyright notice in the Description page of Project Settings.

#include "TankMainWeaponComponent.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "Components/StaticMeshComponent.h"

#include "Projectile.h"



// Sets default values for this component's properties
UTankMainWeaponComponent::UTankMainWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetTickGroup(ETickingGroup::TG_StartPhysics);
}



void UTankMainWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UTankMainWeaponComponent::TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction)
{
	Super::TickComponent(deltaTime, tickType, thisTickFunction);

	auto bWeaponStateChanged = false;

	// Reload gun if needed
	if (RemainReloadTime > 0)
	{
		RemainReloadTime = FMath::Clamp<float>(RemainReloadTime - deltaTime, 0, ReloadTime);
		bWeaponStateChanged = true;
	}

	RotateTurret();
	ElevateBarrel();
	
	// Check if barrel is within desired lock angle tolerance. Update the flag
	if(bTargetLockedOn != (FMath::Abs(FVector::DotProduct(DesiredWorldAimingDirection, Barrel->GetForwardVector())) > FMath::Cos(FMath::DegreesToRadians(AimingLockAngleTolerance))))
	{
		bTargetLockedOn = !bTargetLockedOn;
		bWeaponStateChanged = true;
	}

	if (bWeaponStateChanged)
	{
		OnMainWeaponStateChange.Broadcast(RemainReloadTime, ReloadTime, bTargetLockedOn);
	}
}


#pragma region PRIVATE

void UTankMainWeaponComponent::ElevateBarrel ()
{
	// Find the delta between desired local pitch and current local pitch
	// by projecting the desired world direction to the local pitch plane.
	auto targetFiringLocalPitchDirection = DesiredWorldAimingDirection - FVector::DotProduct(Barrel->GetRightVector(), DesiredWorldAimingDirection) * Barrel->GetRightVector();
	targetFiringLocalPitchDirection.Normalize();
	const auto deltaPitch = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(targetFiringLocalPitchDirection, Barrel->GetForwardVector())));

	// Correct deltaPitch direction
	const auto correctedDeltaPitch = FVector::DotProduct(DesiredWorldAimingDirection, Barrel->GetUpVector()) > 0
		? deltaPitch
		: -deltaPitch;
	
	// Clamp the delta pitch for this frame
	const auto deltaBarrelElevationSpeed = BarrelElevationSpeed * GetWorld()->DeltaTimeSeconds;
	const auto clampedDeltaPitch = FMath::Clamp<float>(correctedDeltaPitch, -deltaBarrelElevationSpeed, deltaBarrelElevationSpeed);

	// Clamp the local pitch to gun's current depression/elevation limit
	const auto currentGunDepressionLimit = GunDepressionAngleCurve ? GunDepressionAngleCurve->GetFloatValue(Turret->RelativeRotation.Yaw) : 0;
	const auto clampedNewLocalPitch = FMath::Clamp<float>(Barrel->RelativeRotation.Pitch + clampedDeltaPitch, currentGunDepressionLimit, GunElevationAngle);

	Barrel->SetRelativeRotation(FRotator(clampedNewLocalPitch, 0, 0));
}

void UTankMainWeaponComponent::RotateTurret ()
{
	// Find the delta between desired local yaw and current local yaw
	// by projecting the desired world direction to the local yaw plane.
	auto targetFiringLocalYawDirection = DesiredWorldAimingDirection - FVector::DotProduct(Turret->GetUpVector(), DesiredWorldAimingDirection) * Turret->GetUpVector();
	targetFiringLocalYawDirection.Normalize();
	const auto deltaYaw = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(targetFiringLocalYawDirection, Turret->GetForwardVector())));

	// Correct deltaYaw direction
	auto correctedDeltaYaw = FVector::DotProduct(DesiredWorldAimingDirection, Turret->GetRightVector()) > 0
		? deltaYaw
		: -deltaYaw;

	// Clamp the delta yaw for this frame
	const auto deltaTurretRotationSpeed = TurretRotationSpeed * GetWorld()->DeltaTimeSeconds;
	const auto clampedDeltaYaw = FMath::Clamp<float>(correctedDeltaYaw, -deltaTurretRotationSpeed, deltaTurretRotationSpeed);

	Turret->AddRelativeRotation(FRotator(0, clampedDeltaYaw, 0));
}

#pragma endregion PRIVATE


#pragma region PUBLIC

void UTankMainWeaponComponent::Init(UStaticMeshComponent * turret, UStaticMeshComponent * barrel)
{
	checkf(turret, TEXT("Failed to init MainWeaponComponent. Turret == nullptr"));
	checkf(barrel, TEXT("Failed to init MainWeaponComponent. Barrel == nullptr"));

	Turret = turret;
	Barrel = barrel;
}

void UTankMainWeaponComponent::AimGun(const FVector & targetLocation, const bool bDrawDebug)
{
	// Check aim solution
	// Use straight line between target and firing location if no solution
	if (!UGameplayStatics::SuggestProjectileVelocity(this, DesiredWorldAimingDirection, Barrel->GetComponentLocation(), targetLocation
		, ProjectileSpeed
		, false, 0, 0, ESuggestProjVelocityTraceOption::DoNotTrace
		, FCollisionResponseParams::DefaultResponseParam
		, TArray<AActor *>()
		, bDrawDebug))
	{
		DesiredWorldAimingDirection = targetLocation - Barrel->GetComponentLocation();
	}

	DesiredWorldAimingDirection.Normalize();
}

bool UTankMainWeaponComponent::TryFireGun()
{
	if (RemainReloadTime > 0) return false;
	
	if(ensureMsgf(Projectile, TEXT("No Projectile specified.")))
	{
		auto projectile = GetWorld()->SpawnActor<AProjectile>(Projectile
			, Barrel->GetComponentLocation() + Barrel->GetForwardVector() * FiringPositionOffset
			, Barrel->GetComponentRotation());

		projectile->SetLifeSpan(ProjectileLifeTimeSec);

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Projectile.GetDefaultObject()->GetMuzzleBlastVfx()
			, Barrel->GetComponentLocation() + Barrel->GetForwardVector() * FiringEffectPositionOffset
			, Barrel->GetComponentRotation());

		RemainReloadTime = ReloadTime;

		return true;
	}

	return false;
}

void UTankMainWeaponComponent::ChangeShellType(TSubclassOf<AProjectile> newShellType)
{
	if(newShellType)
	{
		Projectile = newShellType;
		RemainReloadTime = ReloadTime = newShellType->GetDefaultObject<AProjectile>()->ReloadTime;
		ProjectileSpeed = newShellType->GetDefaultObject<AProjectile>()->GetSpeed();
		ProjectileLifeTimeSec = newShellType->GetDefaultObject<AProjectile>()->LifeTimeSec;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Trying to set invalid projectile."), *GetName());
	}
	
}

bool UTankMainWeaponComponent::CheckIsTargetInAim(AActor * target) const
{
	auto outResult = FPredictProjectilePathResult();
	UGameplayStatics::PredictProjectilePath(this, FPredictProjectilePathParams(1
		, Barrel->GetComponentLocation() + Barrel->GetForwardVector() * FiringPositionOffset
		, Barrel->GetForwardVector() * ProjectileSpeed
		, ProjectileLifeTimeSec
		, ECC_WorldDynamic), outResult);

	return target == outResult.HitResult.Actor.Get();
}

#pragma endregion PUBLIC



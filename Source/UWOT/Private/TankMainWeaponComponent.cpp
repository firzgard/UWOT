// Fill out your copyright notice in the Description page of Project Settings.

#include "TankMainWeaponComponent.h"

#include "Projectile.h"

#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


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

	if(bBlueprintInitialized)
	{
		auto bWeaponStateChanged = false;

		// Reload gun if needed
		if (RemainReloadTime > 0)
		{
			RemainReloadTime = FMath::Clamp<float>(RemainReloadTime - deltaTime, 0, ReloadTime);
			bWeaponStateChanged = true;

			
			// Play reload gun before reload is about to complete
			// Only play on player controlled tank
			if(Cast<APlayerController>(Cast<APawn>(GetOwner())->GetController()) && !bReloadCompleteSfxPlayed && ReloadCompleteSFX && ReloadCompleteSFXPreStartOffsetSec > RemainReloadTime)
			{
				UGameplayStatics::PlaySoundAtLocation(this, ReloadCompleteSFX, Turret->GetComponentLocation());
				bReloadCompleteSfxPlayed = true;
			}
		}

		// Adjust gun if still aiming
		if (!bLockGun && !bAimingCompleted)
		{
			AdjustTurretRotation();
			AdjustBarrelElevation();

			if (TurretRotateAudioComponent && !bTurretRotateSfxPlaying)
			{
				bTurretRotateSfxPlaying = true;
				TurretRotateAudioComponent->Play();
			}
		}
		else
		{
			if (TurretRotateAudioComponent && bTurretRotateSfxPlaying)
			{
				bTurretRotateSfxPlaying = false;
				TurretRotateAudioComponent->FadeOut(TurretRotationSfxFadeOutTime, 0);
			}
		}

		// Check if aiming is completed, with consider for the lock angle tolerance. Update flag if neccessary
		if (!bLockGun && bAimingCompleted != (FMath::Abs(DesiredWorldAimingDirection | Barrel->GetForwardVector()) > FMath::Cos(FMath::DegreesToRadians(AimingCompletedAngleTolerance))))
		{
			bAimingCompleted = !bAimingCompleted;
			bWeaponStateChanged = true;
		}

		if (bWeaponStateChanged)
		{
			OnMainWeaponStateChange.Broadcast(RemainReloadTime, ReloadTime, bAimingCompleted);
		}
	}
}


#pragma region PRIVATE

void UTankMainWeaponComponent::AdjustBarrelElevation()
{
	// Find the delta between desired local pitch and current local pitch
	// by projecting the desired world direction to the local pitch plane.
	const auto targetFiringLocalPitchDirection = FVector::VectorPlaneProject(DesiredWorldAimingDirection, Barrel->GetRightVector()).GetSafeNormal();
	const auto deltaPitch = FMath::RadiansToDegrees(FMath::Acos(targetFiringLocalPitchDirection | Barrel->GetForwardVector()));

	// Correct deltaPitch direction
	const auto correctedDeltaPitch = (DesiredWorldAimingDirection | Barrel->GetUpVector()) > 0 ? deltaPitch : -deltaPitch;
	
	// Clamp the delta pitch for this frame
	const auto deltaBarrelElevationSpeed = BarrelElevationSpeed * GetWorld()->DeltaTimeSeconds;
	const auto clampedDeltaPitch = FMath::Clamp<float>(correctedDeltaPitch, -deltaBarrelElevationSpeed, deltaBarrelElevationSpeed);

	// Clamp the local pitch to gun's current depression/elevation limit
	const auto currentGunDepressionLimit = GunDepressionAngleCurve.GetRichCurveConst()->Eval(Turret->RelativeRotation.Yaw);
	const auto clampedNewLocalPitch = FMath::Clamp<float>(Barrel->RelativeRotation.Pitch + clampedDeltaPitch, currentGunDepressionLimit, GunElevationAngle);

	Barrel->SetRelativeRotation(FRotator(clampedNewLocalPitch, 0, 0));
}

void UTankMainWeaponComponent::AdjustTurretRotation()
{
	// Find the delta between desired local yaw and current local yaw
	// by projecting the desired world direction to the local yaw plane.
	const auto targetFiringLocalYawDirection = FVector::VectorPlaneProject(DesiredWorldAimingDirection, Turret->GetUpVector()).GetSafeNormal();
	const auto deltaYaw = FMath::RadiansToDegrees(FMath::Acos(targetFiringLocalYawDirection | Turret->GetForwardVector()));

	// Correct deltaYaw direction
	const auto correctedDeltaYaw = (DesiredWorldAimingDirection | Turret->GetRightVector()) > 0 ? deltaYaw : -deltaYaw;

	// Clamp the delta yaw for this frame
	const auto deltaTurretRotationSpeed = TurretRotationSpeed * GetWorld()->DeltaTimeSeconds;
	const auto clampedDeltaYaw = FMath::Clamp<float>(correctedDeltaYaw, -deltaTurretRotationSpeed, deltaTurretRotationSpeed);

	Turret->AddRelativeRotation(FRotator(0, clampedDeltaYaw, 0));
}

#pragma endregion PRIVATE


#pragma region PUBLIC

void UTankMainWeaponComponent::Init(UStaticMeshComponent * turret, UStaticMeshComponent * barrel)
{
	checkf(turret, TEXT("%s: Failed to init MainWeaponComponent. Turret == nullptr"), *GetOwner()->GetName());
	checkf(barrel, TEXT("%s: Failed to init MainWeaponComponent. Barrel == nullptr"), *GetOwner()->GetName());

	Turret = turret;
	Barrel = barrel;

	bBlueprintInitialized = true;
}

void UTankMainWeaponComponent::SetTurretRotateAudioComponent(UAudioComponent* turretAudio)
{
	TurretRotateAudioComponent = turretAudio;
}

void UTankMainWeaponComponent::AimGun(const FVector & targetLocation, const bool bDrawDebug)
{
	if(bBlueprintInitialized)
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
}

bool UTankMainWeaponComponent::TryFireGun()
{
	if (RemainReloadTime > 0) return false;
	
	if(ensureMsgf(Projectile, TEXT("No Projectile specified.")))
	{
		auto projectile = GetWorld()->SpawnActor<AProjectile>(Projectile
			, Barrel->GetComponentLocation() + Barrel->GetForwardVector() * FiringPositionOffset
			, Barrel->GetComponentRotation());

		projectile->Fire(GetOwner());

		// Add recoil
		Turret->AddImpulseAtLocation(-Barrel->GetForwardVector() * Projectile.GetDefaultObject()->RecoilImpulse, Barrel->GetComponentLocation());

		Reload();

		return true;
	}

	return false;
}

void UTankMainWeaponComponent::ChangeShellType(TSubclassOf<AProjectile> newShellType)
{
	if(newShellType)
	{
		Projectile = newShellType;
		ProjectileSpeed = newShellType->GetDefaultObject<AProjectile>()->GetSpeed();
		ProjectileLifeTimeSec = newShellType->GetDefaultObject<AProjectile>()->LifeTimeSec;

		ReloadTime = newShellType->GetDefaultObject<AProjectile>()->ReloadTime;
		Reload();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Trying to set invalid projectile."), *GetName());
	}
	
}

void UTankMainWeaponComponent::Reload()
{
	RemainReloadTime = ReloadTime;
	bReloadCompleteSfxPlayed = false;
}

void UTankMainWeaponComponent::TraceProjectilePath(FPredictProjectilePathResult & outResult) const
{
	if (bBlueprintInitialized)
	{
		UGameplayStatics::PredictProjectilePath(this, FPredictProjectilePathParams(1
			, Barrel->GetComponentLocation() + Barrel->GetForwardVector() * FiringPositionOffset
			, Barrel->GetForwardVector() * ProjectileSpeed
			, ProjectileLifeTimeSec
			, ECC_WorldDynamic), outResult);
	}
}

FVector UTankMainWeaponComponent::GetTurretForwardVector() const
{
	return bBlueprintInitialized ? Turret->GetForwardVector() : FVector::ForwardVector;
}

#pragma endregion PUBLIC



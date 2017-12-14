// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "CollisionQueryParams.h"

#include "TankMainWeaponComponent.generated.h"

class AProjectile;

UENUM(BlueprintType)
enum class EMainWeaponState : uint8
{
	Aiming				UMETA(DisplayName = "Aiming"),
	Reloading			UMETA(DisplayName = "Reloading"),
	LockedAndLoaded		UMETA(DisplayName = "LockedAndLoaded"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMainWeaponStateChangeDelegate, EMainWeaponState, state, float, remainTime, float, reloadTime);

/*
 * Tank aiming and firing handler
 */
UCLASS( ClassGroup=(UWOT), meta=(BlueprintSpawnableComponent) )
class UWOT_API UTankMainWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	float BarrelDeltaPitch = 0; // Barrel elevation to be add in this tick
	float TurretDeltaYaw = 0; // Turret rotation to be add in this tick

protected:
	UStaticMeshComponent * Turret = nullptr;
	UStaticMeshComponent * Barrel = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Components|Main Gun Properties")
		float ProjectileSpeed = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Components|Main Gun Properties")
		TSubclassOf<AProjectile> Projectile = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "Components|Main Gun Properties")
		float RemainReloadTime = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Components|Main Gun Properties")
		float ReloadTime = 3;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Main Gun Properties")
		float GunElevationAngle = 20;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Main Gun Properties")
		UCurveFloat* GunDepressionAngleCurve = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Main Gun Properties")
		float BarrelElevationSpeed = 30;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Main Gun Properties")
		float TurretRotationSpeed = 60;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Components|Main Gun Properties")
		EMainWeaponState WeaponState = EMainWeaponState::Reloading;
	UPROPERTY(BlueprintAssignable, Category = "Components|Main Gun Properties")
		FMainWeaponStateChangeDelegate OnMainWeaponStateChange;

private:
	void ElevateBarrel(const FVector & targetBarrelWorldDirection);
	void RotateTurret(const FVector & targetTurretWorldDirection);

protected:
	void BeginPlay() override;
	void TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction) override;

public:

	// Sets default values for this component's properties
	UTankMainWeaponComponent();

	UFUNCTION(BlueprintCallable)
		void Init(UStaticMeshComponent * turret, UStaticMeshComponent * barrel);

	UFUNCTION(BlueprintCallable)
		void AimGun(FVector const & targetLocation
			, const ESuggestProjVelocityTraceOption::Type traceOption = ESuggestProjVelocityTraceOption::DoNotTrace
			, const bool bDrawDebug = false);

	UFUNCTION(BlueprintCallable)
		bool TryFireGun();

	UFUNCTION(BlueprintCallable)
		void ChangeShellType(TSubclassOf<AProjectile> newShellType);
};

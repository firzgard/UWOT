// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "TankMainWeaponComponent.generated.h"

class UCurveFloat;
class UStaticMeshComponent;
class AProjectile;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMainWeaponStateChangeDelegate, float, remainTime, float, reloadTime, bool, bTargetLockedOn);

/*
 * Tank aiming and firing handler
 */
UCLASS(ClassGroup=(UWOT), meta=(BlueprintSpawnableComponent))
class UWOT_API UTankMainWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UStaticMeshComponent * Turret = nullptr;
	UStaticMeshComponent * Barrel = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Components|Main Gun Properties")
		FVector DesiredWorldAimingDirection = FVector::OneVector;
	UPROPERTY(BlueprintReadOnly, Category = "Components|Main Gun Properties")
		float ProjectileSpeed = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Components|Main Gun Properties")
		float ProjectileLifeTimeSec = 3;
	UPROPERTY(BlueprintReadOnly, Category = "Components|Main Gun Properties")
		TSubclassOf<AProjectile> Projectile = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Main Gun Properties")
		float GunElevationAngle = 20;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Main Gun Properties")
		UCurveFloat * GunDepressionAngleCurve = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Main Gun Properties")
		float BarrelElevationSpeed = 30;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Main Gun Properties")
		float TurretRotationSpeed = 60;
	/** The spawn position of projectile will be offset forward from barrel's transform by this much cm */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Main Gun Properties")
		float FiringPositionOffset = 300;
	/** The spawn effect position of projectile will be offset forward from barrel's transform by this much cm */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Main Gun Properties")
		float FiringEffectPositionOffset = 500;

	/** Largest angle in degree between desired aiming angle and real barrel angle that aiming can be considered locked */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Main Gun Properties")
		float AimingLockAngleTolerance = 0.01;

public:
	/** Current shell type's reload time. Readonly */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Main Gun Properties")
		float ReloadTime = 3;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Main Gun Properties")
		float RemainReloadTime = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Main Gun Properties")
		bool bTargetLockedOn = false;
	UPROPERTY(BlueprintAssignable, Category = "Components|Main Gun Properties")
		FMainWeaponStateChangeDelegate OnMainWeaponStateChange;

private:
	void ElevateBarrel();
	void RotateTurret();

protected:
	void BeginPlay() override;
	void TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction) override;

public:

	// Sets default values for this component's properties
	UTankMainWeaponComponent();

	FVector GetTurretForwardVector() const;

	UFUNCTION(BlueprintCallable)
		void Init(UStaticMeshComponent * turret, UStaticMeshComponent * barrel);

	UFUNCTION(BlueprintCallable)
		void AimGun(FVector const & targetLocation, const bool bDrawDebug = false);

	UFUNCTION(BlueprintCallable)
		bool TryFireGun();

	UFUNCTION(BlueprintCallable)
		void ChangeShellType(TSubclassOf<AProjectile> newShellType);

	UFUNCTION(BlueprintCallable)
		void TraceProjectilePath(FPredictProjectilePathResult & outResult) const;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "CollisionQueryParams.h"

#include "TankMainWeaponComponent.generated.h"

class AProjectile;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FReloadGunDelegate, float, remainTime, float, reloadTime);


/*
 * Tank aiming and firing handler
 */
UCLASS( ClassGroup=(UWOT), meta=(BlueprintSpawnableComponent) )
class UWOT_API UTankMainWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UStaticMeshComponent * Turret = nullptr;
	UStaticMeshComponent * Barrel = nullptr;
	USceneComponent * FiringStartPosition = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Components|Main Gun Properties")
		float ProjectileSpeed = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Components|Main Gun Properties")
		float ReloadTime = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Components|Main Gun Properties")
		float RemainReloadTime = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Main Gun Properties")
		float GunElevationAngle = 20;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Main Gun Properties")
		UCurveFloat* GunDepressionAngleCurve = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Main Gun Properties")
		float BarrelElevationSpeed = 30;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Main Gun Properties")
		float TurretRotationSpeed = 60;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components|Main Gun Properties")
		TSubclassOf<AProjectile> Projectile = nullptr;

public:
	UPROPERTY(BlueprintAssignable, Category = "Components|Main Gun Properties")
		FReloadGunDelegate OnReloadGun;

private:
	void ElevateGun(const FVector & targetBarrelWorldDirection);
	void RotateTurret(const FVector & targetTurretWorldDirection);

protected:
	void BeginPlay() override;
	void TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction) override;

public:

	// Sets default values for this component's properties
	UTankMainWeaponComponent();

	UFUNCTION(BlueprintCallable)
		void Init(UStaticMeshComponent * turret, UStaticMeshComponent * barrel, USceneComponent * firingPosition);

	UFUNCTION(BlueprintCallable)
		void AimGun(FVector const & targetLocation
			, const ESuggestProjVelocityTraceOption::Type traceOption = ESuggestProjVelocityTraceOption::DoNotTrace
			, const bool bDrawDebug = false);

	UFUNCTION(BlueprintCallable)
		bool TryFireGun();

	UFUNCTION(BlueprintCallable)
		void ChangeShellType(TSubclassOf<AProjectile> newShellType);
};

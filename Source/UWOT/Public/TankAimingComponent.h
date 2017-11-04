// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "CollisionQueryParams.h"

#include "TankAimingComponent.generated.h"

class AProjectile;

/*
 * Tank aiming and firing handler
 */
UCLASS( ClassGroup=(UWOT), meta=(BlueprintSpawnableComponent) )
class UWOT_API UTankAimingComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	float ShellVelocity = 0;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gun Properties")
		float GunElevationAngle = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gun Properties")
		UCurveFloat* GunDepressionAngleCurve = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gun Properties")
		float BarrelElevationSpeed = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gun Properties")
		float TurretRotationSpeed = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Shell Properties")
		TSubclassOf<AProjectile> Projectile = nullptr;

	UStaticMeshComponent * Turret = nullptr;
	UStaticMeshComponent * Barrel = nullptr;
	USceneComponent * FiringStartPosition = nullptr;

private:
	void ElevateGun(const FVector & targetBarrelWorldDirection);
	void RotateTurret(const FVector & targetTurretWorldDirection);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:

	// Sets default values for this component's properties
	UTankAimingComponent();

	// Called every frame
	virtual void TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction) override;

	UFUNCTION(BlueprintCallable)
		void Init(UStaticMeshComponent * turret, UStaticMeshComponent * barrel, USceneComponent * firingPosition);

	UFUNCTION(BlueprintCallable)
		void AimGun(FVector const & targetLocation
			, const ESuggestProjVelocityTraceOption::Type traceOption = ESuggestProjVelocityTraceOption::DoNotTrace
			, const bool bDrawDebug = false);

	UFUNCTION(BlueprintCallable)
		void FireGun();
};

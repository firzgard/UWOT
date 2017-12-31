// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UParticleSystem;
class UProjectileMovementComponent;
class URadialForceComponent;

UCLASS()
class UWOT_API AProjectile : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Components|Projectile Properties")
		UStaticMeshComponent* CollisionMesh = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Projectile Properties")
		UProjectileMovementComponent * ProjectileMovementComponent = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Projectile Properties")
		URadialForceComponent * ExplosionForceComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Projectile Properties|FXs")
		UParticleSystem * ImpactBlastVfx = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Projectile Properties|FXs")
		UParticleSystem * MuzzleBlastVfx = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Projectile Properties|Meta")
		int FullDamage = 1000;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Projectile Properties|Meta")
		float FullDamageRadius = 100;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Projectile Properties|Meta")
		float DamageFalloff = 1;
public:	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Projectile Properties|Meta")
		float ReloadTime = 2;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Projectile Properties|Meta")
		float LifeTimeSec = 10;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Projectile Properties|Meta")
		float RecoilImpulse = 10000000;
	UPROPERTY(BlueprintReadOnly, Category = "Components|Projectile Properties|Meta")
		AActor * ProjectileOwner = nullptr;

protected:
	void BeginPlay() override;
	void Tick(float deltaTime) override;
	void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
public:
	// Sets default values for this actor's properties
	AProjectile();

	float GetSpeed() const;
	UParticleSystem * GetMuzzleBlastVfx() const;
};

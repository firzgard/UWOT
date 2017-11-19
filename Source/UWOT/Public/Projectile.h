// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UProjectileMovementComponent;

UCLASS()
class UWOT_API AProjectile : public AActor
{
	GENERATED_BODY()

protected:	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Projectile Properties")
		UProjectileMovementComponent * ProjectileMovementComponent = nullptr;

protected:
	void BeginPlay() override;
	void Tick(float deltaTime) override;

public:	
	// Sets default values for this actor's properties
	AProjectile();

	float GetSpeed() const;
};

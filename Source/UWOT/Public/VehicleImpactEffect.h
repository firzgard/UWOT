// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VehicleImpactEffect.generated.h"

class USoundCue;

/*
* Spawnable effect for vehicle impact - NOT replicated to clients
*/
UCLASS(Abstract, Blueprintable)
class AVehicleImpactEffect : public AActor
{
	GENERATED_UCLASS_BODY()

		/** default impact FX used when material specific override doesn't exist */
		UPROPERTY(EditDefaultsOnly, Category = Defaults)
		UParticleSystem* DefaultFX;

	/** impact FX on asphalt */
	UPROPERTY(EditDefaultsOnly, Category = Visual)
		UParticleSystem* AsphaltFX;

	/** impact FX on dirt */
	UPROPERTY(EditDefaultsOnly, Category = Visual)
		UParticleSystem* DirtFX;

	/** impact FX on water */
	UPROPERTY(EditDefaultsOnly, Category = Visual)
		UParticleSystem* WaterFX;

	/** impact FX on wood */
	UPROPERTY(EditDefaultsOnly, Category = Visual)
		UParticleSystem* WoodFX;

	/** impact FX on stone */
	UPROPERTY(EditDefaultsOnly, Category = Visual)
		UParticleSystem* StoneFX;

	/** impact FX on metal */
	UPROPERTY(EditDefaultsOnly, Category = Visual)
		UParticleSystem* MetalFX;

	/** impact FX on grass */
	UPROPERTY(EditDefaultsOnly, Category = Visual)
		UParticleSystem* GrassFX;

	/** impact FX on gravel */
	UPROPERTY(EditDefaultsOnly, Category = Visual)
		UParticleSystem* GravelFX;

	/** default impact sound used when material specific override doesn't exist */
	UPROPERTY(EditDefaultsOnly, Category = Defaults)
		USoundCue* DefaultSound;

	/** impact sound on asphalt */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
		USoundCue* AsphaltSound;

	/** impact sound on dirt */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
		USoundCue* DirtSound;

	/** impact sound on water */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
		USoundCue* WaterSound;

	/** impact sound on wood */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
		USoundCue* WoodSound;

	/** impact sound on stone */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
		USoundCue* StoneSound;

	/** impact sound on metal */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
		USoundCue* MetalSound;

	/** impact sound on grass */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
		USoundCue* GrassSound;

	/** impact sound on gravel */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
		USoundCue* GravelSound;

	/** impact sound when landing on wheels */
	UPROPERTY(EditDefaultsOnly, Category = Defaults)
		USoundCue* WheelLandingSound;

	/** surface data for spawning */
	UPROPERTY(BlueprintReadOnly, Category = Impact)
		FHitResult HitSurface;

	/** impact force */
	UPROPERTY(BlueprintReadOnly, Category = Impact)
		FVector HitForce;

	/** whether impact was coming from landing on wheels (otherwise - hit with body) */
	bool bWheelLand;

	/** spawn effect */
	virtual void PostInitializeComponents() override;

protected:
	/** get FX for material type */
	UParticleSystem* GetImpactFX(TEnumAsByte<EPhysicalSurface> MaterialType);

	/** get sound for material type */
	USoundCue* GetImpactSound(TEnumAsByte<EPhysicalSurface> MaterialType);
};

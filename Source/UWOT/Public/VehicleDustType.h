// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "Particles/ParticleSystem.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#include "VehicleDustType.generated.h"

/*
* Configuration of dust effects shown under wheels
*/
UCLASS()
class UVehicleDustType : public UDataAsset
{
	GENERATED_UCLASS_BODY()

	/** FX under wheel on asphalt */
	UPROPERTY(EditDefaultsOnly, Category = Effect)
	UParticleSystem* AsphaltFX;

	/** FX under wheel on dirt */
	UPROPERTY(EditDefaultsOnly, Category = Effect)
		UParticleSystem* DirtFX;

	/** FX under wheel on water */
	UPROPERTY(EditDefaultsOnly, Category = Effect)
		UParticleSystem* WaterFX;

	/** FX under wheel on grass */
	UPROPERTY(EditDefaultsOnly, Category = Effect)
		UParticleSystem* GrassFX;

	/** FX under wheel on gravel */
	UPROPERTY(EditDefaultsOnly, Category = Effect)
		UParticleSystem* GravelFX;

	/** min speed to show FX on asphalt */
	UPROPERTY(EditDefaultsOnly, Category = Speed)
		float AsphaltMinSpeed;

	/** min speed to show FX on dirt */
	UPROPERTY(EditDefaultsOnly, Category = Speed)
		float DirtMinSpeed;

	/** min speed to show FX on water */
	UPROPERTY(EditDefaultsOnly, Category = Speed)
		float WaterMinSpeed;

	/** min speed to show FX on grass */
	UPROPERTY(EditDefaultsOnly, Category = Speed)
		float GrassMinSpeed;

	/** min speed to show FX on gravel */
	UPROPERTY(EditDefaultsOnly, Category = Speed)
		float GravelMinSpeed;

	/** determine correct FX */
	UParticleSystem* GetDustFX(UPhysicalMaterial* PhysMaterial, float CurrentSpeed);

protected:

	/** get FX for material type */
	UParticleSystem* GetWheelFX(TEnumAsByte<EPhysicalSurface> MaterialType);

	/** get min speed for material type */
	float GetMinSpeed(TEnumAsByte<EPhysicalSurface> MaterialType);
};

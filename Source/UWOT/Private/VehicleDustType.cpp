// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "VehicleDustType.h"

#include "UWOT.h"
#include "GlobalTypes.h"

UVehicleDustType::UVehicleDustType(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

UParticleSystem* UVehicleDustType::GetDustFX(UPhysicalMaterial* PhysMaterial, float CurrentSpeed)
{
	EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(PhysMaterial);

	// determine particle FX
	UParticleSystem* WheelFX = GetWheelFX(SurfaceType);
	if (WheelFX)
	{
		const float MinSpeed = GetMinSpeed(SurfaceType);
		if (CurrentSpeed < MinSpeed)
		{
			WheelFX = nullptr;
		}
	}

	return WheelFX;
}

UParticleSystem* UVehicleDustType::GetWheelFX(TEnumAsByte<EPhysicalSurface> MaterialType)
{
	UParticleSystem* WheelFX = nullptr;

	switch (MaterialType)
	{
	case VEHICLE_SURFACE_Asphalt:		WheelFX = AsphaltFX; break;
	case VEHICLE_SURFACE_Dirt:			WheelFX = DirtFX; break;
	case VEHICLE_SURFACE_Water:			WheelFX = WaterFX; break;
	case VEHICLE_SURFACE_Grass:			WheelFX = GrassFX; break;
	case VEHICLE_SURFACE_Gravel:		WheelFX = GravelFX; break;
	default:							WheelFX = nullptr; break;
	}

	return WheelFX;
}

float UVehicleDustType::GetMinSpeed(TEnumAsByte<EPhysicalSurface> MaterialType)
{
	float MinSpeed = 0.0f;

	switch (MaterialType)
	{
	case VEHICLE_SURFACE_Asphalt:		MinSpeed = AsphaltMinSpeed; break;
	case VEHICLE_SURFACE_Dirt:			MinSpeed = DirtMinSpeed; break;
	case VEHICLE_SURFACE_Water:			MinSpeed = WaterMinSpeed; break;
	case VEHICLE_SURFACE_Grass:			MinSpeed = GrassMinSpeed; break;
	case VEHICLE_SURFACE_Gravel:		MinSpeed = GravelMinSpeed; break;
	default:							MinSpeed = 0.0f; break;
	}

	return MinSpeed;
}

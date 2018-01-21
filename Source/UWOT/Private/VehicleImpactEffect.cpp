// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "VehicleImpactEffect.h"

#include "UWOT.h"
#include "GlobalTypes.h"

#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AVehicleImpactEffect::AVehicleImpactEffect(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bAutoDestroyWhenFinished = true;
}

void AVehicleImpactEffect::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	const auto hitSurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitSurface.PhysMaterial.Get());

	// show particles
	const auto impactFx = GetImpactFX(hitSurfaceType);
	if (impactFx)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, impactFx, GetActorLocation(), GetActorRotation());
	}

	// play sound
	auto impactSound = bWheelLand ? WheelLandingSound : GetImpactSound(hitSurfaceType);
	if (impactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, impactSound, GetActorLocation());
	}
}

UParticleSystem* AVehicleImpactEffect::GetImpactFX(TEnumAsByte<EPhysicalSurface> MaterialType)
{
	UParticleSystem* ImpactFX = nullptr;

	switch (MaterialType)
	{
	case VEHICLE_SURFACE_Asphalt:	ImpactFX = AsphaltFX; break;
	case VEHICLE_SURFACE_Dirt:		ImpactFX = DirtFX; break;
	case VEHICLE_SURFACE_Water:		ImpactFX = WaterFX; break;
	case VEHICLE_SURFACE_Wood:		ImpactFX = WoodFX; break;
	case VEHICLE_SURFACE_Stone:		ImpactFX = StoneFX; break;
	case VEHICLE_SURFACE_Metal:		ImpactFX = MetalFX; break;
	case VEHICLE_SURFACE_Grass:		ImpactFX = GrassFX; break;
	case VEHICLE_SURFACE_Gravel:	ImpactFX = GravelFX; break;
	default:							ImpactFX = nullptr; break;
	}

	return ImpactFX ? ImpactFX : DefaultFX;
}

USoundCue* AVehicleImpactEffect::GetImpactSound(TEnumAsByte<EPhysicalSurface> MaterialType)
{
	USoundCue* ImpactSound = nullptr;

	switch (MaterialType)
	{
	case VEHICLE_SURFACE_Asphalt:	ImpactSound = AsphaltSound; break;
	case VEHICLE_SURFACE_Dirt:		ImpactSound = DirtSound; break;
	case VEHICLE_SURFACE_Water:		ImpactSound = WaterSound; break;
	case VEHICLE_SURFACE_Wood:		ImpactSound = WoodSound; break;
	case VEHICLE_SURFACE_Stone:		ImpactSound = StoneSound; break;
	case VEHICLE_SURFACE_Metal:		ImpactSound = MetalSound; break;
	case VEHICLE_SURFACE_Grass:		ImpactSound = GrassSound; break;
	case VEHICLE_SURFACE_Gravel:	ImpactSound = GravelSound; break;
	default:							ImpactSound = nullptr; break;
	}

	return ImpactSound ? ImpactSound : DefaultSound;
}

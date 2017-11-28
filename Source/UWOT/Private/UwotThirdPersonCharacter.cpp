// Fill out your copyright notice in the Description page of Project Settings.

#include "UwotThirdPersonCharacter.h"

void AUwotThirdPersonCharacter::BeginPlay()
{
	Super::BeginPlay();

	CamouflageRemainDuration = CamouflageDuration;
	ReceiveChangeCamouflage(CamouflageVisibility);
}

void AUwotThirdPersonCharacter::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	// Eat away camo energy if camouflaging
	if (bCamouflage)
	{
		CamouflageRemainDuration -= deltaTime;
		CamouflageRemainDuration = FMath::Clamp<float>(CamouflageRemainDuration, 0, CamouflageDuration);
		
		// DeCamo if run out of energy
		if (CamouflageRemainDuration == 0)
		{
			bCamouflage = false;
		}
	}
	// Recover camo energy gauge if not camouflaging
	else if (CamouflageRemainDuration < CamouflageDuration)
	{
		CamouflageRemainDuration += deltaTime;
		CamouflageRemainDuration = FMath::Clamp<float>(CamouflageRemainDuration, 0, CamouflageDuration);
	}

	// Change camouflage ammout if visibility is not corresponding with camouflage stage
	if ((bCamouflage && CamouflageVisibility != 0)
		|| (!bCamouflage && CamouflageVisibility != 1))
	{
		CamouflageVisibility += deltaTime / (bCamouflage ? -EnCamouflageTime : DeCamouflageTime);
		CamouflageVisibility = FMath::Clamp<float>(CamouflageVisibility, 0, 1);

		// Call the blueprint anim event
		ReceiveChangeCamouflage(CamouflageVisibility);
	}
}

bool AUwotThirdPersonCharacter::TrySetCamouflage(bool bEnable)
{
	if (CamouflageRemainDuration > 0)
	{
		bCamouflage = bEnable;

		return true;
	}

	return false;
}
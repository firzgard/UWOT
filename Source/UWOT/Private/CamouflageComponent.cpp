// Fill out your copyright notice in the Description page of Project Settings.

#include "CamouflageComponent.h"


// Sets default values for this component's properties
UCamouflageComponent::UCamouflageComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UCamouflageComponent::BeginPlay()
{
	Super::BeginPlay();

	CamouflageRemainDuration = CamouflageDuration;
	OnChangeCamouflage.Broadcast(CamouflageFactor);
}


// Called every frame
void UCamouflageComponent::TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction)
{
	Super::TickComponent(deltaTime, tickType, thisTickFunction);

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
	if ((bCamouflage && CamouflageFactor != 1)
		|| (!bCamouflage && CamouflageFactor != 0))
	{
		CamouflageFactor += deltaTime / (bCamouflage ? EnCamouflageTime : -DeCamouflageTime);
		CamouflageFactor = FMath::Clamp<float>(CamouflageFactor, 0, 1);

		// Call the blueprint anim event
		OnChangeCamouflage.Broadcast(CamouflageFactor);
	}
}

bool UCamouflageComponent::TrySetCamouflage(bool bEnable)
{
	if (CamouflageRemainDuration > 0)
	{
		bCamouflage = bEnable;

		return true;
	}

	return false;
}

void UCamouflageComponent::DepletAll()
{
	bCamouflage = false;
	CamouflageRemainDuration = 0;
}

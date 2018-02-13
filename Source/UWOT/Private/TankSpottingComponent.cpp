// Fill out your copyright notice in the Description page of Project Settings.

#include "TankSpottingComponent.h"

#include "TankSpottingManager.h"

#include "GameFramework/Pawn.h"

UTankSpottingComponent::UTankSpottingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UTankSpottingComponent::BeginPlay()
{
	Super::BeginPlay();

	ATankSpottingManager::GetInstance()->SubscribeSpottingComponenent(this);

	CurrentBaseCamouflageFactor = DesiredBaseCamouflageFactor = CurrentCamouflageFactor = BaseCamouflageFactor;
}

void UTankSpottingComponent::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	ATankSpottingManager::GetInstance()->UnsubscribeSpottingComponenent(this);

	Super::EndPlay(endPlayReason);
}

void UTankSpottingComponent::TickComponent(float DeltaTime
	, ELevelTick TickType
	, FActorComponentTickFunction* ThisTickFunction)
{
	CalculateCamoFactor(DeltaTime);

	// Reset bHasFired if already reached required camouflage factor value
	if(bHasFired && OnFiringBaseCamouflageFactor == CurrentBaseCamouflageFactor)
	{
		bHasFired = false;
	}
}

void UTankSpottingComponent::CalculateCamoFactor(const float deltaTime)
{
	const static float STATION_VELOCITY_THRESTHOLD_SQUARED = 1000;

	if(bHasFired)
	{
		DesiredBaseCamouflageFactor = OnFiringBaseCamouflageFactor;
	}
	else
	{
		DesiredBaseCamouflageFactor = GetOwner()->GetVelocity().SizeSquared() > STATION_VELOCITY_THRESTHOLD_SQUARED ? OnMovingBaseCamouflageFactor : BaseCamouflageFactor;
	}

	if (DesiredBaseCamouflageFactor > CurrentBaseCamouflageFactor)
	{
		CurrentBaseCamouflageFactor = FMath::Clamp(CurrentBaseCamouflageFactor + deltaTime * BaseCamouflageRecoverRate, 0.0f, DesiredBaseCamouflageFactor);
	}
	else if (DesiredBaseCamouflageFactor < CurrentBaseCamouflageFactor)
	{
		CurrentBaseCamouflageFactor = FMath::Clamp(CurrentBaseCamouflageFactor - deltaTime * BaseCamouflageDropRate, DesiredBaseCamouflageFactor, 1.0f);
	}

	

	CurrentCamouflageFactor = FMath::Clamp(CurrentBaseCamouflageFactor + InInvisibleModeCamouflageFactorBonus * InvisibleModeBonusFactor, 0.0f, 1.0f);
}


void UTankSpottingComponent::Init(TArray<USceneComponent*> newSpottingPorts, TArray<USceneComponent*> newDetectionPorts)
{
	SpottingPorts = newSpottingPorts;
	DetectionPorts = newDetectionPorts;
}


void UTankSpottingComponent::OnSpottedSelf(const ETankTeamEnum teamId, const bool bSpotted)
{
	bIsSpotted = bSpotted;

	// Add custom, non-controller-related tank-spotted code here

	if(auto controller = Cast<APawn>(GetOwner())->GetController())
	{
		if(auto tankController = Cast<ITankController>(controller))
		{
			tankController->Execute_OnSpottedSelf(controller, bIsSpotted);
		}
	}
}

void UTankSpottingComponent::OnSpottedOther(const UTankSpottingComponent * other, const bool bSpotted)
{
	// Add custom, non-controller-related tank-spotted code here

	if (auto controller = Cast<APawn>(GetOwner())->GetController())
	{
		if (auto tankController = Cast<ITankController>(controller))
		{
			tankController->Execute_OnSpottedOther(controller, bSpotted, Cast<ATank>(other->GetOwner()));
		}
	}
}
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InterfaceTankController.h"

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "TankSpottingComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UWOT_API UTankSpottingComponent : public UActorComponent
{
GENERATED_BODY()

private:
	float CurrentBaseCamouflageFactor = 0;
	float DesiredBaseCamouflageFactor = 0;
	float CurrentCamouflageFactor = 0;
	float InvisibleModeBonusFactor = 0;

public:
	/** ID to differentiate tank's team during spotting processing */
	UPROPERTY(BlueprintReadOnly)
		ETankTeamEnum TeamId = ETankTeamEnum::NONE;
	UPROPERTY(BlueprintReadOnly)
		bool bIsSpotted = false;

	UPROPERTY(BlueprintReadWrite)
		bool bHasFired = false;

	UPROPERTY(BlueprintReadOnly, Category = Mechanics)
		TArray<USceneComponent *> SpottingPorts;
	UPROPERTY(BlueprintReadOnly, Category = Mechanics)
		TArray<USceneComponent *> DetectionPorts;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Meta)
		float BaseViewRange = 50000;
	/* Range at which spotting is 100%, regardless of base view range */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Meta)
		float AutoSpotRange = 5000;
	/** Checking interval, in second*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Meta)
		float SpottingIntervalSec = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Meta)
		float BaseCamouflageFactor = 0.2;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Meta)
		float OnMovingBaseCamouflageFactor = 0.1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Meta)
		float OnFiringBaseCamouflageFactor = 0.05;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Meta)
		float InInvisibleModeCamouflageFactorBonus = 0.6;

	/** Speed at which camo factor would recovered per second */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Meta)
		float BaseCamouflageRecoverRate = 0.025;
	/** Speed at which camo factor would dropped per second */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Meta)
		float BaseCamouflageDropRate = 0.1;

private:
	void CalculateCamoFactor(float deltaTime);

protected:
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:	
	UTankSpottingComponent();

	void OnSpottedSelf(const ETankTeamEnum teamId, const bool bSpotted);
	void OnSpottedOther(const UTankSpottingComponent * other, const bool bSpotted);

	UFUNCTION(BlueprintCallable)
		void SetInvisibleModeFactor(const float rawFactor) { InvisibleModeBonusFactor = FMath::Clamp(rawFactor, 0.0f, 1.0f); }

	UFUNCTION(BlueprintCallable)
		float GetCurrentCamouflageFactor() const { return CurrentCamouflageFactor; }

	UFUNCTION(BlueprintCallable)
		void Init(TArray<USceneComponent *> newSpottingPorts, TArray<USceneComponent *> newDetectionPorts);
};

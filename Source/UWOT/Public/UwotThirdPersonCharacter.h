// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TP_ThirdPerson/TP_ThirdPersonCharacter.h"
#include "UwotThirdPersonCharacter.generated.h"

/**
 * 
 */
UCLASS()
class UWOT_API AUwotThirdPersonCharacter : public ATP_ThirdPersonCharacter
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float CamouflageRemainDuration = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float EnCamouflageTime = 3;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float DeCamouflageTime = .5f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float CamouflageDuration = 30;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float CamouflageFullRecoverTime = 10;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bCamouflage = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float CamouflageVisibility = 1;

protected:
	void BeginPlay() override;
	void Tick(float deltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Change Camouflage"))
		void ReceiveChangeCamouflage(float visibility);

	UFUNCTION(BlueprintCallable)
		bool TrySetCamouflage(bool bEnable);
};

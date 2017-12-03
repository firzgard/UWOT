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
	void BeginPlay() override;
	void Tick(float deltaTime) override;
};

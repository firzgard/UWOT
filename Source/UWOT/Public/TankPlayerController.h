// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Tank.h"
#include "InterfaceTankController.h"

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "TankPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class UWOT_API ATankPlayerController : public APlayerController, public ITankController
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly)
		ETankTeamEnum TeamId = ETankTeamEnum::TEAM_1;

public:
	UPROPERTY(BlueprintReadOnly)
		ATank * ControlledTank;

protected:
	void BeginPlay() override;
	void SetPawn(APawn* InPawn) override;
	void Tick(float deltaTime) override;

	void OnSpottedSelf_Implementation(bool bSpotted) override;
	void OnSpottedOther_Implementation(bool bSpotted, ATank * other) override;

public:
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "SetTank"))
		void ReceiveSetTank(ATank * lastTank);

	UFUNCTION(BlueprintCallable)
		void GetAimingTargetPosition(FVector const &CursorWorldLocation, FVector const &CursorWorldDirection, float const LineTraceRange, FVector& OutTargetPosition) const;
	UFUNCTION(BlueprintCallable)
		ETankTeamEnum GetTeamId() override;
	UFUNCTION(BlueprintCallable)
		ATank * GetControlledTank() override;
};

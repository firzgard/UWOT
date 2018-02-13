// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Tank.h"
#include "InterfaceTankController.h"

#include "CoreMinimal.h"
#include "AIController.h"
#include "TankAIController.generated.h"

/**
 * 
 */
UCLASS()
class UWOT_API ATankAIController : public AAIController, public ITankController
{
	GENERATED_BODY()

private:
	ATank * TargetTank = nullptr;
	FVector LastSpottedTargetLocation;
	bool bHasTarget = false;

protected:
	UPROPERTY(BlueprintReadOnly)
		ETankTeamEnum TeamId = ETankTeamEnum::TEAM_2;

public:
	UPROPERTY(BlueprintReadOnly)
		ATank * ControlledTank;

public: 
	/** How far will the AI tank move toward player before stopping */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|AI Properties")
		float AcceptanceDistance = 3000;
	/** At which angle should the AI tank face player to increase effective armor */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|AI Properties")
		float BestAngleDeg = 30;
	/** Largest angle in degree between best angle and real angle that is considered acceptable */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|AI Properties")
		float BestAngleToleranceDeg = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Logging Properties")
		bool bDrawAimingDebugLine = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|AI Properties")
		bool bFirable = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|AI Properties")
		bool bMovable = true;

protected:
	void OnSpottedSelf_Implementation(bool bSpotted) override;
	void OnSpottedOther_Implementation(bool bSpotted, ATank * other) override;

public:

	void BeginPlay() override;
	void SetPawn(APawn* InPawn) override;
	void Tick(float deltaTime) override;
	float TakeDamage(float damage, FDamageEvent const& damageEvent, AController* eventInstigator, AActor* damageCauser) override;

	UFUNCTION(BlueprintCallable)
		ETankTeamEnum GetTeamId() override;
	UFUNCTION(BlueprintCallable)
		ATank * GetControlledTank() override;
};

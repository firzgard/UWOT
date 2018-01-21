// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UWOT.h"
#include "WheeledVehicleMovementComponent.h"
#include "WheeledVehicleMovementComponent4W.h"
#include "TankMovementComponent.generated.h"

class UTrackComponent;

// TODO: Implement realistic tank steering
UENUM()
enum class ETankSteering : uint8
{
	ClutchBraking,
	TwinTranmission,
	DoubleDifferential
};

USTRUCT()
struct FReplicatedTankState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
		float LeftThrustInput;
	UPROPERTY()
		float RightThrustInput;
	UPROPERTY()
		float BrakeInputLeft;
	UPROPERTY()
		float BrakeInputRight;
};

UCLASS(ClassGroup = (UWOT), meta = (BlueprintSpawnableComponent))
class UWOT_API UTankMovementComponent : public UWheeledVehicleMovementComponent
{
	GENERATED_BODY()

protected:
	UTrackComponent * LeftTrack = nullptr;
	UTrackComponent * RightTrack = nullptr;

	UPROPERTY(Transient, Replicated)
		FReplicatedTankState ReplicatedTankState;
	UPROPERTY(Transient)
		float LeftThrustInput;
	UPROPERTY(Transient)
		float RightThrustInput;
	UPROPERTY(Transient)
		float BrakeLeftInput;
	UPROPERTY(Transient)
		float BrakeRightInput;

	// How much engine torque does player want to diverse to the left. Range -1...1
	UPROPERTY(Transient)
		float RawLeftThrustInput;
	// How much engine torque does player want to diverse to the right. Range -1...1
	UPROPERTY(Transient)
		float RawRightThrustInput;
	// What the player has the right brake set to. Range -1...1
	UPROPERTY(Transient)
		float RawBrakeRightInput;
	// What the player has the left brake set to. Range -1...1
	UPROPERTY(Transient)
		float RawBrakeLeftInput;

	// How much to press the brake when the player has release throttle
	UPROPERTY(EditAnywhere, Category = VehicleInput, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
		float IdleBrakeInputLeft;
	// How much to press the brake when the player has release throttle
	UPROPERTY(EditAnywhere, Category = VehicleInput, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
		float IdleBrakeInputRight;

	// Left auto-brake input. Range 0...1. Set to 0 to disable.
	UPROPERTY(EditAnywhere, Category = VehicleInput, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
		float AutoLeftBrakeInput = 1;
	// Right auto-brake input. Range 0...1. Set to 0 to disable.
	UPROPERTY(EditAnywhere, Category = VehicleInput, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
		float AutoRightBrakeInput = 1;

	// Rate at which thrust input for left caterpillar can rise and fall
	UPROPERTY(EditAnywhere, Category = VehicleInput)
		FVehicleInputRate LeftThrustChangeRate;
	// Rate at which thrust input for right caterpillar can rise and fall
	UPROPERTY(EditAnywhere, Category = VehicleInput)
		FVehicleInputRate RightThrustChangeRate;

	// Rate at which brake input for left caterpillar can rise and fall
	UPROPERTY(EditAnywhere, Category = VehicleInput)
		FVehicleInputRate BrakeLeftRate;
	// Rate at which brake input can for right caterpillar rise and fall
	UPROPERTY(EditAnywhere, Category = VehicleInput)
		FVehicleInputRate BrakeRightRate;

public:
	UTankMovementComponent(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Category = MechanicalSetup)
		FVehicleEngineData EngineSetup;

	UPROPERTY(EditAnywhere, Category = MechanicalSetup)
		FVehicleTransmissionData TransmissionSetup;

	UPROPERTY(EditAnywhere, Category = MechanicalSetup)
		int LeftSprocketWheelIndex = 0;

	UPROPERTY(EditAnywhere, Category = MechanicalSetup)
		int RightSprocketWheelIndex = 1;

	/** Maximum steering versus forward speed (km/h) */
	UPROPERTY(EditAnywhere, Category = SteeringSetup)
		FRuntimeFloatCurve SteeringCurve;

	/* Left thrust input mapped with steering angle.
	 * Thrust input is within -1..1
	 * Steering angle is within -90..90, clockwise */
	UPROPERTY(EditAnywhere, Category = SteeringSetup)
		FRuntimeFloatCurve LeftThrustSteeringAngleMappingCurve;

	/* Right thrust input mapped with steering angle.
	* Thrust input is within -1..1
	* Steering angle is within -90..90, clockwise */
	UPROPERTY(EditAnywhere, Category = SteeringSetup)
		FRuntimeFloatCurve RightThrustSteeringAngleMappingCurve;

	/* Left brake input mapped with steering angle.
	* Thrust input is within 0..1
	* Steering angle is within -90..90, clockwise */
	UPROPERTY(EditAnywhere, Category = SteeringSetup)
		FRuntimeFloatCurve LeftBrakeSteeringAngleMappingCurve;

	/* Right brake input mapped with steering angle.
	* Thrust input is within 0..1
	* Steering angle is within -90..90, clockwise */
	UPROPERTY(EditAnywhere, Category = SteeringSetup)
		FRuntimeFloatCurve RightBrakeSteeringAngleMappingCurve;

private:
	/** Read current state for simulation */
	void UpdateTankState(float DeltaTime);

protected:

	void PreTick(float DeltaTime) override;
	void TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction) override;

	#if WITH_EDITOR
		void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	#endif

	void RequestDirectMove(const FVector& moveVelocity, bool bForceMaxSpeed) override;
	void SetupVehicle() override;
	void UpdateSimulation(float DeltaTime) override;

	/** update simulation data: engine */
	void UpdateEngineSetup(const FVehicleEngineData& newEngineSetup);
	/** update simulation data: transmission */
	void UpdateTransmissionSetup(const FVehicleTransmissionData& newGearSetup);

	UFUNCTION(Reliable, Server, WithValidation)
		void ServerUpdateTankState(float newLeftThrustInput, float newRightThrustInput, float newLeftBrakeInput, float newRightBrakeInput);


public:
	void Serialize(FArchive & Ar) override;
	void ComputeConstants() override;
	/** Clear all interpolated control inputs */
	void ClearInput() override;

	UFUNCTION(BlueprintCallable, Category = "Setup")
		void Init(UTrackComponent * leftTrackInstance, UTrackComponent * rightTrackInstace);

	/** Calls Torque On Left Caterpillar*/
	UFUNCTION(BlueprintCallable, Category = "Game|Components|TankMovement")
		void SetLeftThrustInput(float newLeftThrust);

	/** Calls Torque On Right Caterpillar*/
	UFUNCTION(BlueprintCallable, Category = "Game|Components|TankMovement")
		void SetRightThrustInput(float newRightThrust);

	/** Calls Break On Left Caterpillar*/
	UFUNCTION(BlueprintCallable, Category = "Game|Components|TankMovement")
		void SetRightBrakeInput(float newBrakeRight);

	/** Calls Break On Right Caterpillar*/
	UFUNCTION(BlueprintCallable, Category = "Game|Components|TankMovement")
		void SetLeftBrakeInput(float newBrakeLeft);

	UFUNCTION(BlueprintCallable, Category = "Game|Components|TankMovement")
		void SetSteeringDirection(FVector2D desiredSteeringDirection);

	UFUNCTION(BlueprintCallable, Category = "Game|Components|TankMovement")
		float GetThrottleInput() const;
};

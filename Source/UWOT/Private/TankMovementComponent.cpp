// Fill out your copyright notice in the Description page of Project Settings.

#include "TankMovementComponent.h"

#include "Components/PrimitiveComponent.h"
#include "GameFramework/Pawn.h"
#include "PhysXIncludes.h"
#include "PhysXPublic.h"
#include "UnrealNetwork.h"

#include "TrackComponent.h"


#pragma region HELPERS

	// BUG?? FVehicleEngineData::FindPeakTorque in WheeledVehicleMovementComponent4W.cpp is not linked
	float FVehicleEngineData::FindPeakTorque() const
	{
		// Find max torque
		float PeakTorque = 0.f;
		TArray<FRichCurveKey> TorqueKeys = TorqueCurve.GetRichCurveConst()->GetCopyOfKeys();
		for (int32 KeyIdx = 0; KeyIdx < TorqueKeys.Num(); KeyIdx++)
		{
			FRichCurveKey& Key = TorqueKeys[KeyIdx];
			PeakTorque = FMath::Max(PeakTorque, Key.Value);
		}
		return PeakTorque;
	}

	// Copy UWheeledVehicleMovementComponent4W BackwardsConvertCm2ToM2()
	static void BackwardsConvertCm2ToM2(float& val, const float defaultValue)
	{
		if (val != defaultValue)
		{
			val = Cm2ToM2(val);
		}
	}

	// Copy UWheeledVehicleMovementComponent4W GetVehicleEngineSetup()
	static void GetVehicleEngineSetup(const FVehicleEngineData& setup, PxVehicleEngineData& pxSetup)
	{
		pxSetup.mMOI = M2ToCm2(setup.MOI);
		pxSetup.mMaxOmega = RPMToOmega(setup.MaxRPM);
		pxSetup.mDampingRateFullThrottle = M2ToCm2(setup.DampingRateFullThrottle);
		pxSetup.mDampingRateZeroThrottleClutchEngaged = M2ToCm2(setup.DampingRateZeroThrottleClutchEngaged);
		pxSetup.mDampingRateZeroThrottleClutchDisengaged = M2ToCm2(setup.DampingRateZeroThrottleClutchDisengaged);

		const auto peakTorque = setup.FindPeakTorque(); // In Nm
		pxSetup.mPeakTorque = M2ToCm2(peakTorque); // convert Nm to (kg cm^2/s^2)

		// Convert from our curve to PhysX
		pxSetup.mTorqueCurve.clear();
		auto torqueKeys = setup.TorqueCurve.GetRichCurveConst()->GetCopyOfKeys();
		const auto numTorqueCurveKeys = FMath::Min<int32>(torqueKeys.Num(), PxVehicleEngineData::eMAX_NB_ENGINE_TORQUE_CURVE_ENTRIES);
		for (auto keyIdx = 0; keyIdx < numTorqueCurveKeys; keyIdx++)
		{
			auto& key = torqueKeys[keyIdx];
			pxSetup.mTorqueCurve.addPair(FMath::Clamp(key.Time / setup.MaxRPM, 0.f, 1.f), key.Value / peakTorque); // Normalize torque to 0-1 range
		}
	}

	// Copy UWheeledVehicleMovementComponent4W GetVehicleGearSetup()
	// TODO: Support multi reverse gears
	static void GetVehicleGearSetup(const FVehicleTransmissionData& Setup, PxVehicleGearsData& PxSetup)
	{
		PxSetup.mSwitchTime = Setup.GearSwitchTime;
		PxSetup.mRatios[PxVehicleGearsData::eREVERSE] = Setup.ReverseGearRatio;
		for (int32 i = 0; i < Setup.ForwardGears.Num(); i++)
		{
			PxSetup.mRatios[i + PxVehicleGearsData::eFIRST] = Setup.ForwardGears[i].Ratio;
		}
		PxSetup.mFinalRatio = Setup.FinalRatio;
		PxSetup.mNbRatios = Setup.ForwardGears.Num() + PxVehicleGearsData::eFIRST;
	}

	// Copy UWheeledVehicleMovementComponent4W GetVehicleAutoBoxSetup()
	// TODO: Support multi reverse gears
	static void GetVehicleAutoBoxSetup(const FVehicleTransmissionData& Setup, PxVehicleAutoBoxData& PxSetup)
	{
		for (auto i = 0; i < Setup.ForwardGears.Num(); i++)
		{
			const auto& gearData = Setup.ForwardGears[i];
			PxSetup.mUpRatios[i + PxVehicleGearsData::eFIRST] = gearData.UpRatio;
			PxSetup.mDownRatios[i + PxVehicleGearsData::eFIRST] = gearData.DownRatio;
		}
		PxSetup.mUpRatios[PxVehicleGearsData::eNEUTRAL] = Setup.NeutralGearUpRatio;
		PxSetup.setLatency(Setup.GearAutoBoxLatency);
	}

	static void SetupDriveHelper(const UTankMovementComponent* vehicleData, PxVehicleDriveSimData& DriveData)
	{
		PxVehicleEngineData engineSetup;
		GetVehicleEngineSetup(vehicleData->EngineSetup, engineSetup);
		DriveData.setEngineData(engineSetup);

		PxVehicleClutchData clutchSetup;
		clutchSetup.mStrength = M2ToCm2(vehicleData->TransmissionSetup.ClutchStrength);
		DriveData.setClutchData(clutchSetup);

		PxVehicleGearsData gearSetup;
		GetVehicleGearSetup(vehicleData->TransmissionSetup, gearSetup);
		DriveData.setGearsData(gearSetup);

		PxVehicleAutoBoxData autoBoxSetup;
		GetVehicleAutoBoxSetup(vehicleData->TransmissionSetup, autoBoxSetup);
		DriveData.setAutoBoxData(autoBoxSetup);
	}

	static float CalculateBrakeInput(float rawBrakeInput, float autoBrakeInput, float idleBrakeInput
		, float forwardSpeed, float rawThrottleInput, float rawThrustInput
		, float wrongDirectionThreshold, float autoStopThresholdSpeed)
	{
		// if player wants to move forward
		// and if vehicle is moving backwards
		if (rawThrottleInput > 0.f && forwardSpeed < -wrongDirectionThreshold)
		{
			rawBrakeInput = autoBrakeInput;
		}

		// if player wants to move backwards
		// and if vehicle is moving forwards
		else if (rawThrottleInput < 0.f && forwardSpeed > wrongDirectionThreshold)
		{
			rawBrakeInput = autoBrakeInput;
		}

		// if player isn't pressing forward or backwards
		else if (rawThrottleInput == 0)
		{
			rawBrakeInput = idleBrakeInput;
		}

		return FMath::Clamp<float>(rawBrakeInput, 0.0, 1.0);
	}

	// Calc thrust input with RVO avoidance
	static float CalculateThrustInput(float rawThrustInput, float rvoThrottleStep, float avoidanceSpeedSquared, float desiredAvoidanceSpeedSquared)
	{
		if (avoidanceSpeedSquared > desiredAvoidanceSpeedSquared)
		{
			rawThrustInput = FMath::Clamp(rawThrustInput + rvoThrottleStep, -1.0f, 1.0f);
		}
		else if (avoidanceSpeedSquared < desiredAvoidanceSpeedSquared)
		{
			rawThrustInput = FMath::Clamp(rawThrustInput - rvoThrottleStep, -1.0f, 1.0f);
		}

		return rawThrustInput;
	}

#pragma endregion HELPERS


#pragma region UE_LIFECYCLE

UTankMovementComponent::UTankMovementComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// grab default values from physx
	PxVehicleEngineData defEngineData;
	EngineSetup.MOI = defEngineData.mMOI;
	EngineSetup.MaxRPM = OmegaToRPM(defEngineData.mMaxOmega);
	EngineSetup.DampingRateFullThrottle = defEngineData.mDampingRateFullThrottle;
	EngineSetup.DampingRateZeroThrottleClutchEngaged = defEngineData.mDampingRateZeroThrottleClutchEngaged;
	EngineSetup.DampingRateZeroThrottleClutchDisengaged = defEngineData.mDampingRateZeroThrottleClutchDisengaged;

	// Convert from PhysX curve to Unreal Engine's
	auto torqueCurveData = EngineSetup.TorqueCurve.GetRichCurve();
	for (PxU32 keyIdx = 0; keyIdx < defEngineData.mTorqueCurve.getNbDataPairs(); keyIdx++)
	{
		const auto input = defEngineData.mTorqueCurve.getX(keyIdx) * EngineSetup.MaxRPM;
		const auto output = defEngineData.mTorqueCurve.getY(keyIdx) * defEngineData.mPeakTorque;
		torqueCurveData->AddKey(input, output);
	}

	const PxVehicleClutchData defClutchData;
	TransmissionSetup.ClutchStrength = defClutchData.mStrength;

	const PxVehicleGearsData defGearSetup;
	TransmissionSetup.GearSwitchTime = defGearSetup.mSwitchTime;
	TransmissionSetup.ReverseGearRatio = defGearSetup.mRatios[PxVehicleGearsData::eREVERSE];
	TransmissionSetup.FinalRatio = defGearSetup.mFinalRatio;

	PxVehicleAutoBoxData defAutoBoxSetup;
	TransmissionSetup.NeutralGearUpRatio = defAutoBoxSetup.mUpRatios[PxVehicleGearsData::eNEUTRAL];
	TransmissionSetup.GearAutoBoxLatency = defAutoBoxSetup.getLatency();
	TransmissionSetup.bUseGearAutoBox = true;

	for (uint32 i = PxVehicleGearsData::eFIRST; i < defGearSetup.mNbRatios; i++)
	{
		FVehicleGearData gearData;
		gearData.DownRatio = defAutoBoxSetup.mDownRatios[i];
		gearData.UpRatio = defAutoBoxSetup.mUpRatios[i];
		gearData.Ratio = defGearSetup.mRatios[i];
		TransmissionSetup.ForwardGears.Add(gearData);
	}

	// Init steering curves
	{
		auto steeringCurveData = SteeringCurve.GetRichCurve();
		steeringCurveData->AddKey(0.f, 1.f);
		steeringCurveData->AddKey(20.f, 0.9f);
		steeringCurveData->AddKey(60.f, 0.8f);
		steeringCurveData->AddKey(120.f, 0.7f);

		auto trackSpeedDifferentialSteeringAngleMappingCurveData = TrackSpeedDifferentialSteeringAngleMappingCurve.GetRichCurve();
		trackSpeedDifferentialSteeringAngleMappingCurveData->AddKey(-90, -1);
		trackSpeedDifferentialSteeringAngleMappingCurveData->AddKey(0, 1);
		trackSpeedDifferentialSteeringAngleMappingCurveData->AddKey(90, -1);
	}

	// Initialize WheelSetups array with selected number of wheels
	WheelSetups.SetNum(4);

	// Setup Default Values for Input Rate
	IdleBrakeInputLeft = IdleBrakeInputRight = IdleBrakeInput;
	BrakeLeftRate.RiseRate = BrakeRightRate.RiseRate = BrakeInputRate.RiseRate;
	BrakeLeftRate.FallRate = BrakeRightRate.FallRate = BrakeInputRate.FallRate;
	LeftThrustChangeRate.RiseRate = RightThrustChangeRate.RiseRate = SteeringInputRate.RiseRate;
	LeftThrustChangeRate.FallRate = RightThrustChangeRate.FallRate = SteeringInputRate.FallRate;
}

void UTankMovementComponent::PreTick(float DeltaTime)
{
	Super::PreTick(DeltaTime);

	if (PVehicle && UpdatedComponent)
	{
		if (Cast<APawn>(UpdatedComponent->GetOwner()))
		{
			UpdateTankState(DeltaTime);
		}
	}
}

void UTankMovementComponent::TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction)
{
	// Update track animation
	for (auto i = 0; i < Wheels.Num(); i++)
	{
		if (i % 2 == 0)
		{
			if (LeftTrack)
			{
				LeftTrack->AdjustSpline(i, Wheels[i]->Location, Wheels[i]->ShapeRadius);
			}
		}
		else
		{
			if (RightTrack)
			{
				RightTrack->AdjustSpline(i, Wheels[i]->Location, Wheels[i]->ShapeRadius);
			}
		}
	}

	if(LeftTrack)
	{
		LeftTrack->AnimateTrack(PVehicle->mWheelsDynData.getWheelRotationSpeed(0) * deltaTime * Wheels[LeftSprocketWheelIndex]->ShapeRadius);
	}
	if (RightTrack)
	{
		RightTrack->AnimateTrack(PVehicle->mWheelsDynData.getWheelRotationSpeed(1) * deltaTime * Wheels[RightSprocketWheelIndex]->ShapeRadius);
	}
}

#if WITH_EDITOR
void UTankMovementComponent::PostEditChangeProperty(struct FPropertyChangedEvent& propertyChangedEvent)
{
	Super::PostEditChangeProperty(propertyChangedEvent);

	// Validate changed value if property is one of following
	const UProperty * propertyThatChanged = propertyChangedEvent.MemberProperty;
	if (propertyThatChanged)
	{
		const auto propertyName = propertyThatChanged->GetFName();

		if (propertyName == GET_MEMBER_NAME_CHECKED(FVehicleGearData, DownRatio))
		{
			// Correct DownRatio <= UpRatio
			for (auto gearId = 0; gearId < TransmissionSetup.ForwardGears.Num(); ++gearId)
			{
				auto & gearData = TransmissionSetup.ForwardGears[gearId];
				gearData.DownRatio = FMath::Min(gearData.DownRatio, gearData.UpRatio);
			}
		}
		else if (propertyName == GET_MEMBER_NAME_CHECKED(FVehicleGearData, UpRatio))
		{
			// Correct UpRatio >= DownRatio
			for (auto gearId = 0; gearId < TransmissionSetup.ForwardGears.Num(); ++gearId)
			{
				auto & gearData = TransmissionSetup.ForwardGears[gearId];
				gearData.UpRatio = FMath::Max(gearData.DownRatio, gearData.UpRatio);
			}
		}
		else if (propertyName == GET_MEMBER_NAME_CHECKED(UTankMovementComponent, SteeringCurve))
		{
			// Correct SteeringCurve's value to be in range 0 - 1
			auto steerKeys = SteeringCurve.GetRichCurve()->GetCopyOfKeys();
			for (auto keyId = 0; keyId < steerKeys.Num(); ++keyId)
			{
				const auto newValue = FMath::Clamp(steerKeys[keyId].Value, 0.f, 1.f);
				SteeringCurve.GetRichCurve()->UpdateOrAddKey(steerKeys[keyId].Time, newValue);
			}
		}
	}
}
#endif

#pragma endregion UE_LIFECYCLE


#pragma region INTERNAL_INTERFACE

void UTankMovementComponent::UpdateTankState(float DeltaTime)
{
	// update input values
	const auto owner = UpdatedComponent ? Cast<APawn>(UpdatedComponent->GetOwner()) : NULL;

	// TODO: IsLocallyControlled will fail if the owner is unpossessed (i.e. MyOwner->GetController() == nullptr);
	// Should we remove input instead of relying on replicated state in that case?
	if (owner && owner->IsLocallyControlled())
	{			
		if (bUseRVOAvoidance)
		{
			CalculateAvoidanceVelocity(DeltaTime);
			UpdateAvoidance(DeltaTime);
		}

		BrakeLeftInput = BrakeLeftRate.InterpInputValue(DeltaTime, BrakeLeftInput, CalculateBrakeInput(RawBrakeLeftInput, AutoLeftBrakeInput, IdleBrakeInputLeft
			, GetForwardSpeed(), RawThrottleInput, RawLeftThrustInput
			, WrongDirectionThreshold, StopThreshold));
		BrakeRightInput = BrakeRightRate.InterpInputValue(DeltaTime, BrakeRightInput, CalculateBrakeInput(RawBrakeRightInput, AutoRightBrakeInput, IdleBrakeInputRight
			, GetForwardSpeed(), RawThrottleInput, RawRightThrustInput
			, WrongDirectionThreshold, StopThreshold));

		if(bUseRVOAvoidance)
		{
			const auto avoidanceSpeedSquared = AvoidanceVelocity.SizeSquared();
			const auto desiredAvoidanceSpeedSquared = GetVelocityForRVOConsideration().SizeSquared();
			LeftThrustInput = LeftThrustChangeRate.InterpInputValue(DeltaTime, LeftThrustInput, CalculateThrustInput(RawLeftThrustInput, RVOThrottleStep, avoidanceSpeedSquared, desiredAvoidanceSpeedSquared));
			RightThrustInput = RightThrustChangeRate.InterpInputValue(DeltaTime, RightThrustInput, CalculateThrustInput(RawRightThrustInput, RVOThrottleStep, avoidanceSpeedSquared, desiredAvoidanceSpeedSquared));
		}
		else
		{
			LeftThrustInput = LeftThrustChangeRate.InterpInputValue(DeltaTime, LeftThrustInput, RawLeftThrustInput);
			RightThrustInput = RightThrustChangeRate.InterpInputValue(DeltaTime, RightThrustInput, RawRightThrustInput);
		}

		// Prevent auto-switching gear on turning
		if (TransmissionSetup.bUseGearAutoBox && GetCurrentGear() != 0)
		{
			static const auto AUTO_GEAR_OFF_THRUST_DIFF_THRESHOLD = 0.1f;
			if (FMath::IsNearlyEqual(LeftThrustInput, RightThrustInput, AUTO_GEAR_OFF_THRUST_DIFF_THRESHOLD) && BrakeLeftInput == BrakeRightInput)
			{
				if(!GetUseAutoGears())
				{
					SetUseAutoGears(true);
				}
			}
			else
			{
				if(GetUseAutoGears())
				{
					SetUseAutoGears(false);

					if (bReverseAsBrake)
					{
						//for reverse as state we want to automatically shift between reverse and first gear
						if (GetForwardSpeed() < TurnInPlaceSpeedThreshold)	//we only shift between reverse and first if the car is slow enough. This isn't 100% correct since we really only care about engine speed, but good enough
						{
							if (RawThrottleInput < 0.f && GetCurrentGear() >= 0)
							{
								SetTargetGear(-1, true);
							}
							else if (RawThrottleInput > 0.f && GetCurrentGear() <= 0)
							{
								SetTargetGear(1, true);
							}
						}
					}
					else
					{
						SetTargetGear(GetTargetGear(), true);
					}
				}
			}
		}

		ServerUpdateState(SteeringInput, ThrottleInput, BrakeInput, HandbrakeInput, GetCurrentGear());
		ServerUpdateTankState(LeftThrustInput, RightThrustInput, BrakeLeftInput, BrakeRightInput);
	}
	else
	{
		// use replicated values for remote pawns
		LeftThrustInput = ReplicatedTankState.LeftThrustInput;
		RightThrustInput = ReplicatedTankState.RightThrustInput;
		ThrottleInput = ReplicatedState.ThrottleInput;
		BrakeLeftInput = ReplicatedTankState.BrakeInputLeft;
		BrakeRightInput = ReplicatedTankState.BrakeInputRight;
		SetTargetGear(ReplicatedState.CurrentGear, true);
	}

	RawThrottleInput = RawBrakeLeftInput = RawBrakeRightInput = RawLeftThrustInput = RawRightThrustInput = 0;
}

void UTankMovementComponent::RequestDirectMove(const FVector& moveVelocity, bool bForceMaxSpeed)
{
	auto const bForward = (GetOwner()->GetActorForwardVector() | moveVelocity) >= 0;
	SetThrottleInput(bForward ? 1 : -1);
	SetTargetGear(bForward ? TransmissionSetup.ForwardGears.Num() : -1, false);
	SetSteeringDirection(FVector2D(GetOwner()->GetTransform().InverseTransformVector(moveVelocity.GetSafeNormal())));
}

void UTankMovementComponent::SetupVehicle()
{
	// Setup the chassis and wheel shapes
	SetupVehicleShapes();

	// Setup mass properties
	SetupVehicleMass();

	// Setup the wheels
	auto pWheelsSimData = PxVehicleWheelsSimData::allocate(WheelSetups.Num());
	SetupWheels(pWheelsSimData);

	// Setup drive data
	PxVehicleDriveSimData driveData;
	SetupDriveHelper(this, driveData);

	auto pVehicleDriveTank = PxVehicleDriveTank::allocate(WheelSetups.Num());
	check(pVehicleDriveTank);
	pVehicleDriveTank->setDriveModel(PxVehicleDriveTankControlModel::eSPECIAL);

	ExecuteOnPxRigidDynamicReadWrite(UpdatedPrimitive->GetBodyInstance(), [&](PxRigidDynamic* PRigidDynamic)
	{
		pVehicleDriveTank->setup(GPhysXSDK, UpdatedPrimitive->GetBodyInstance()->GetPxRigidDynamic_AssumesLocked(), *pWheelsSimData, driveData, 0);
		pVehicleDriveTank->setToRestState();

		// cleanup
		pWheelsSimData->free();
	});

	// cache values
	PVehicle = pVehicleDriveTank;
	PVehicleDrive = pVehicleDriveTank;

	SetUseAutoGears(TransmissionSetup.bUseGearAutoBox);
}

void UTankMovementComponent::UpdateSimulation(float DeltaTime)
{
	if (PVehicleDrive == nullptr)
		return;

	// TODO: Should we use eStandard for Differential steering and eSpecial for Twin-transmission steering?
	PxVehicleDriveTankRawInputData rawInputData(PxVehicleDriveTankControlModel::eSPECIAL);
	rawInputData.setAnalogAccel(ThrottleInput);
	rawInputData.setAnalogLeftThrust(LeftThrustInput);
	rawInputData.setAnalogRightThrust(RightThrustInput);
	rawInputData.setAnalogLeftBrake(BrakeLeftInput);
	rawInputData.setAnalogRightBrake(BrakeRightInput);

	if (!GetUseAutoGears())
	{
		rawInputData.setGearUp(bRawGearUpInput);
		rawInputData.setGearDown(bRawGearDownInput);
	}

	// Convert from our curve to PxFixedSizeLookupTable
	PxFixedSizeLookupTable<8> speedSteerLookup;
	auto steerKeys = SteeringCurve.GetRichCurve()->GetCopyOfKeys();
	const auto maxSteeringSamples = FMath::Min(8, steerKeys.Num());
	for (auto keyIdx = 0; keyIdx < maxSteeringSamples; keyIdx++)
	{
		auto& key = steerKeys[keyIdx];
		speedSteerLookup.addPair(KmHToCmS(key.Time), FMath::Clamp(key.Value, 0.f, 1.f));
	}

	const PxVehiclePadSmoothingData smoothData = {
		{ ThrottleInputRate.RiseRate, BrakeInputRate.RiseRate, HandbrakeInputRate.RiseRate, SteeringInputRate.RiseRate, SteeringInputRate.RiseRate, LeftThrustChangeRate.RiseRate, RightThrustChangeRate.RiseRate, BrakeLeftRate.RiseRate, BrakeRightRate.RiseRate },
		{ ThrottleInputRate.FallRate, BrakeInputRate.FallRate, HandbrakeInputRate.FallRate, SteeringInputRate.FallRate, SteeringInputRate.FallRate, LeftThrustChangeRate.FallRate, RightThrustChangeRate.FallRate, BrakeLeftRate.FallRate, BrakeRightRate.FallRate }
	};

	const auto pVehicleDriveTank = static_cast<PxVehicleDriveTank*>(PVehicleDrive);
	PxVehicleDriveTankSmoothAnalogRawInputsAndSetAnalogInputs(smoothData, rawInputData, DeltaTime, *pVehicleDriveTank);
}

// Updates engine setup on spawn.
void UTankMovementComponent::UpdateEngineSetup(const FVehicleEngineData& newEngineSetup)
{
	if (PVehicleDrive)
	{
		PxVehicleEngineData engineData;
		GetVehicleEngineSetup(newEngineSetup, engineData);

		auto pVehicleDriveTank =static_cast<PxVehicleDriveTank*>(PVehicleDrive);
		pVehicleDriveTank->mDriveSimData.setEngineData(engineData);
	}
}

// Updates transmisson setup on spawn.
void UTankMovementComponent::UpdateTransmissionSetup(const FVehicleTransmissionData& newTransmissionSetup)
{
	if (PVehicleDrive)
	{
		PxVehicleGearsData gearData;
		GetVehicleGearSetup(newTransmissionSetup, gearData);

		PxVehicleAutoBoxData autoBoxData;
		GetVehicleAutoBoxSetup(newTransmissionSetup, autoBoxData);

		auto pVehicleDriveTank = static_cast<PxVehicleDriveTank*>(PVehicleDrive);
		pVehicleDriveTank->mDriveSimData.setGearsData(gearData);
		pVehicleDriveTank->mDriveSimData.setAutoBoxData(autoBoxData);
	}
}


bool UTankMovementComponent::ServerUpdateTankState_Validate(float newLeftThrustInput, float newRightThrustInput, float newBrakeInputLeft, float newBrakeInputRight)
{
	return true;
}

// Update movement variables on server side.
void UTankMovementComponent::ServerUpdateTankState_Implementation(float newLeftThrustInput, float newRightThrustInput, float newBrakeInputLeft, float newBrakeInputRight)
{
	// update state of inputs
	ReplicatedTankState.LeftThrustInput = newLeftThrustInput;
	ReplicatedTankState.RightThrustInput = newRightThrustInput;
	ReplicatedTankState.BrakeInputLeft = newBrakeInputLeft;
	ReplicatedTankState.BrakeInputRight = newBrakeInputRight;
}

#pragma endregion INTERNAL_INTERFACE



#pragma region PUBLIC_INTERFACE

// Copy UWheeledVehicleMovementComponent4W::Serialize()
void UTankMovementComponent::Serialize(FArchive & Ar)
{
	Super::Serialize(Ar);

	if (Ar.IsLoading() && Ar.UE4Ver() < VER_UE4_VEHICLES_UNIT_CHANGE)
	{
		const PxVehicleEngineData defEngineData;
		const auto defaultRpm = OmegaToRPM(defEngineData.mMaxOmega);

		//we need to convert from old units to new. This backwards compatable code fails in the rare case that they were using very strange values that are the new defaults in the correct units.
		EngineSetup.MaxRPM = EngineSetup.MaxRPM != defaultRpm ? OmegaToRPM(EngineSetup.MaxRPM) : defaultRpm;	//need to convert from rad/s to RPM
	}

	if (Ar.IsLoading() && Ar.UE4Ver() < VER_UE4_VEHICLES_UNIT_CHANGE2)
	{
		const PxVehicleEngineData defEngineData;
		const PxVehicleClutchData defClutchData;

		//we need to convert from old units to new. This backwards compatable code fails in the rare case that they were using very strange values that are the new defaults in the correct units.
		BackwardsConvertCm2ToM2(EngineSetup.DampingRateFullThrottle, defEngineData.mDampingRateFullThrottle);
		BackwardsConvertCm2ToM2(EngineSetup.DampingRateZeroThrottleClutchDisengaged, defEngineData.mDampingRateZeroThrottleClutchDisengaged);
		BackwardsConvertCm2ToM2(EngineSetup.DampingRateZeroThrottleClutchEngaged, defEngineData.mDampingRateZeroThrottleClutchEngaged);
		BackwardsConvertCm2ToM2(EngineSetup.MOI, defEngineData.mMOI);
		BackwardsConvertCm2ToM2(TransmissionSetup.ClutchStrength, defClutchData.mStrength);
	}
}

void UTankMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTankMovementComponent, ReplicatedTankState);
}

// Copy UWheeledVehicleMovementComponent4W::ComputeConstants()
void UTankMovementComponent::ComputeConstants()
{
	Super::ComputeConstants();
	MaxEngineRPM = EngineSetup.MaxRPM;
}

void UTankMovementComponent::ClearInput()
{
	Super::ClearInput();

	BrakeLeftInput = 0;
	BrakeRightInput = 0;
	LeftThrustInput = 0;
	RightThrustInput = 0;

	ServerUpdateTankState(LeftThrustInput, RightThrustInput, BrakeLeftInput, BrakeRightInput);
}



void UTankMovementComponent::Init(UTrackComponent * leftTrack, UTrackComponent * rightTrack)
{
	checkf(leftTrack, TEXT("Failed to init MovementComponent. LeftTract == nullptr"));
	checkf(rightTrack, TEXT("Failed to init MovementComponent. RightTract == nullptr"));

	LeftTrack = leftTrack;
	RightTrack = rightTrack;
}


void UTankMovementComponent::SetLeftThrustInput(const float steeringLeft)
{
	RawLeftThrustInput = FMath::Clamp(steeringLeft, -1.0f, 1.0f);
}

void UTankMovementComponent::SetRightThrustInput(const float steeringRight)
{
	RawRightThrustInput = FMath::Clamp(steeringRight, -1.0f, 1.0f);
}

void UTankMovementComponent::SetLeftBrakeInput(const float newBrakeLeft)
{
	RawBrakeLeftInput = FMath::Clamp(newBrakeLeft, 0.0f, 1.0f);
}

void UTankMovementComponent::SetRightBrakeInput(const float newBrakeRight)
{
	RawBrakeRightInput = FMath::Clamp(newBrakeRight, 0.0f, 1.0f);
}

void UTankMovementComponent::SetSteeringDirection(const FVector2D desiredSteeringDirection)
{
	// Set track's thrust based on steering direction
	if (!desiredSteeringDirection.IsZero())
	{
		const auto steeringAngle = FMath::RadiansToDegrees(FMath::Asin(desiredSteeringDirection.GetSafeNormal().Y));

		auto const leftTrackRotationSpeed = PVehicle->mWheelsDynData.getWheelRotationSpeed(LeftSprocketWheelIndex);
		auto const rightTrackRotationSpeed = PVehicle->mWheelsDynData.getWheelRotationSpeed(RightSprocketWheelIndex);

		auto const desiredTrackRotationSpeedDifference = FMath::Clamp(TrackSpeedDifferentialSteeringAngleMappingCurve.GetRichCurveConst()->Eval(steeringAngle), -1.0f, 1.0f);

		// Steering to right
		if(steeringAngle > 0)
		{
			if(leftTrackRotationSpeed == 0)
			{
				RawLeftThrustInput = 1;
				RawRightThrustInput = 1;
			}
			else
			{
				const auto rightTrackRotationSpeedDifference =  rightTrackRotationSpeed / leftTrackRotationSpeed;

				// Increase/reduce tracks' speed to archieve desired differential
				if (desiredTrackRotationSpeedDifference < rightTrackRotationSpeedDifference)
				{
					RawLeftThrustInput = 1;
					RawRightThrustInput = -1;
				}

				// If already achieved desired differential, stop giving right track thrust
				else
				{
					RawLeftThrustInput = 1;
					RawRightThrustInput = desiredTrackRotationSpeedDifference;
				}
			}
		}

		// Steering to left
		else if (steeringAngle < 0)
		{
			if (rightTrackRotationSpeed == 0)
			{
				RawLeftThrustInput = 1;
				RawRightThrustInput = 1;
			}
			else
			{
				const auto leftTrackRotationSpeedDifference = leftTrackRotationSpeed / rightTrackRotationSpeed;
				// Increase/reduce tracks' speed to archieve desired differential
				if (desiredTrackRotationSpeedDifference < leftTrackRotationSpeedDifference)
				{
					RawLeftThrustInput = -1;
					RawRightThrustInput = 1;
				}
				else
				{
					RawLeftThrustInput = desiredTrackRotationSpeedDifference;
					RawRightThrustInput = 1;
				}
			}
		}

		// No steering
		else
		{
			// Stabilize track speed differencial
			// Increase/reduce tracks' speed to archieve equal rotation speed between 2 track
			{
				// TODO: Should we expose these magic numbers to editor?
				static const auto HORIZONTAL_VEL_TOLERANCE = 20.0f;
				static const auto ADJUSTMENT_THRUST = 0.2f;

				const auto relativeVelocity = GetOwner()->GetTransform().InverseTransformVectorNoScale(GetOwner()->GetVelocity());

				if (FMath::IsNearlyZero(relativeVelocity.Y, HORIZONTAL_VEL_TOLERANCE))
				{
					RawLeftThrustInput = 1;
					RawRightThrustInput = 1;
				}
				else if (relativeVelocity.Y > 0)
				{
					RawLeftThrustInput = 1;
					RawRightThrustInput = ADJUSTMENT_THRUST;
				}
				else
				{
					RawLeftThrustInput = ADJUSTMENT_THRUST;
					RawRightThrustInput = 1;
				}
			}
		}

	}
}

float UTankMovementComponent::GetThrottleInput() const
{
	return ThrottleInput;
}

#pragma endregion PUBLIC_INTERFACE
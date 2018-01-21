// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundNode.h"
#include "VehicleEngineSoundNode.generated.h"

USTRUCT()
struct FVehicleEngineSoundDatum
{
	GENERATED_USTRUCT_BODY()

	/* The FadeInRPM at which to start hearing this sound. */
	UPROPERTY(EditAnywhere, Category = VehicleDatum)
	float FadeInRPMStart;

	/* RPM at which this sound has faded in completely. */
	UPROPERTY(EditAnywhere, Category = VehicleDatum)
		float FadeInRPMEnd;

	/* The FadeOutRPM is where this sound starts fading out. */
	UPROPERTY(EditAnywhere, Category = VehicleDatum)
		float FadeOutRPMStart;

	/* RPM at which this sound is no longer audible. */
	UPROPERTY(EditAnywhere, Category = VehicleDatum)
		float FadeOutRPMEnd;

	/** Pitch multiplier at end of RPM range **/
	UPROPERTY(EditAnywhere, Category = VehicleDatum)
		float MaxPitchMultiplier;

	FVehicleEngineSoundDatum()
		: FadeInRPMStart(0)
		, FadeInRPMEnd(0)
		, FadeOutRPMStart(0)
		, FadeOutRPMEnd(0)
		, MaxPitchMultiplier(1.0f)
	{
	}
};

/**
* Mix and shift pitch of samples depending on engine's RPM
*/
UCLASS(hidecategories = Object, editinlinenew)
class UWOT_API UVehicleEngineSoundNode : public USoundNode
{
	GENERATED_UCLASS_BODY()

public:
	struct FVehicleDesiredRPM
	{
		float DesiredRPM;
		float TimeStamp;
	};

private:

	/** AudioThread authoritative cache of desired RPM keyed by owner ID, reference to generate sound */
	static TMap<uint32, FVehicleDesiredRPM> DesiredRPMDict;

	float CurrentRPMStoreTime;
	float CurrentMaxRPM;

	UPROPERTY()
		float CurrentRPM;

protected:
	/**
	* Each input needs to have the correct data filled in so the SoundNodeVehicleEngine is able
	* to determine which sounds to play
	*/
	UPROPERTY(EditAnywhere, export, editfixedsize, Category = VehicleEngine)
		TArray<FVehicleEngineSoundDatum> EngineSoundSamples;

public:

	// Begin USoundNode interface. 

	void ParseNodes(FAudioDevice* AudioDevice, const UPTRINT NodeWaveInstanceHash, FActiveSound& ActiveSound, const FSoundParseParameters& ParseParams, TArray<FWaveInstance*>& WaveInstances) override;
	int32 GetMaxChildNodes() const override { return MAX_ALLOWED_CHILD_NODES; }
	void CreateStartingConnectors(void) override;
	void InsertChildNode(int32 Index) override;
	void RemoveChildNode(int32 Index) override;

	#if WITH_EDITOR
		/** Ensure amount of Cross Fade inputs matches new amount of children */
		virtual void SetChildNodes(TArray<USoundNode*>& InChildNodes) override;
	#endif // WITH_EDITOR

	// End USoundNode interface. 

	/**
	* Set desired rpm key so that engine sound can be generated for this player. Old datum with same playerId will be replaced
	* Run on Audio thread
	* 
	*/
	static void SetDesiredRPM(int32 playerId, FVehicleDesiredRPM desiredRPM);

	/** 
	* Remove desired rpm to generate engine sound for this player. Old datum with same playerId will be replaced
	* Run on Audio thread
	*/
	static void RemoveDesiredRPM(int32 playerId);

	void StoreCurrentRPM(FAudioDevice* audioDevice, FActiveSound& activeSound, const FSoundParseParameters& parseParams, float maxRPM);

};
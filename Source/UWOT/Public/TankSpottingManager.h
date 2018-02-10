// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TankSpottingManager.generated.h"

enum class ETankTeamEnum : uint8;
struct FSpottingTeamEntry;
class UTankSpottingComponent;


struct UWOT_API FSpottingEntry
{
	/** TimeStamp for next spotting invocation for this component */
	float SpottingTimeStamp = 0;

	/** TimeStamp for despotting this component */
	TMap<ETankTeamEnum, float> DeSpottingTimeStampDict;

	FSpottingEntry(const float spottingTimeStamp)
	{
		SpottingTimeStamp = spottingTimeStamp;
	}
};

struct UWOT_API FSpottingTeamEntry
{
	TMap<UTankSpottingComponent *, FSpottingEntry> SpottingMemberDict;
};


UCLASS()
class UWOT_API ATankSpottingManager : public AActor
{
GENERATED_BODY()

private:
	static ATankSpottingManager * Instance;

	TMap<ETankTeamEnum, FSpottingTeamEntry> SpottingTeamDict;

protected:
	UPROPERTY(BlueprintReadOnly, Category = Meta)
		float SpottingIntervalRandomFactor = 0.1;
	/** Duration since last spotting, after which target will be despotted */
	UPROPERTY(BlueprintReadOnly, Category = Meta)
		float SpottedDuration = 7;

private:
	float CalcRandomSpottingTimeStamp(const float baseInterval) const
	{ 
		return GetWorld()->GetTimeSeconds() + baseInterval * FMath::RandRange(1 - SpottingIntervalRandomFactor, 1 + SpottingIntervalRandomFactor);
	}

	void InvokeSpottingTest(UTankSpottingComponent * spotter);
	bool DoSpottingTest(UTankSpottingComponent * spotter, UTankSpottingComponent * target) const;

protected:
	void PreInitializeComponents() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void Tick(float DeltaTime) override;

public:
	static ATankSpottingManager * GetInstance();

	void SubscribeSpottingComponenent(UTankSpottingComponent * member);
	void UnsubscribeSpottingComponenent(UTankSpottingComponent * member);
};
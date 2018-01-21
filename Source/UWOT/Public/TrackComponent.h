// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "TrackComponent.generated.h"

class UInstancedStaticMeshComponent;

USTRUCT(BlueprintType)
struct FSplinePointBinding
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly)
		TArray<int> PointIds;
};

/**
 * Generated instanced static mesh track
 */
UCLASS(ClassGroup = (UWOT), meta = (BlueprintSpawnableComponent))
class UWOT_API UTrackComponent : public USplineComponent
{
	GENERATED_BODY()

private:
	UInstancedStaticMeshComponent * Mesh = nullptr;

	// Offset by percent of track length, use to roll track
	float TrackOffsetPercentage = 0;

protected:
	UPROPERTY(EditDefaultsOnly, Category = TrackSetup)
		 int TreadCount = 80;

	/** Use this to offset track away from wheel surface */
	UPROPERTY(EditDefaultsOnly, Category = TrackSetup)
		float TrackThicknessOffset = 10;

	/** Bind spline points with tank wheel */
	UPROPERTY(EditAnywhere, Category = TrackSetup)
		TMap<int, FSplinePointBinding> SplinePointBindings;

protected:
	void TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction) override;

public:
	UTrackComponent(const FObjectInitializer& objectInitializer);

	UFUNCTION(BlueprintCallable)
		void BuildTrack(UInstancedStaticMeshComponent * newMesh);

	void AdjustSpline(const int wheelId, const FVector relativeLocation, const float wheelRadius);
	void AnimateTrack(float deltaTrackOffset);
};

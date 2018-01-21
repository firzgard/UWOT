// Fill out your copyright notice in the Description page of Project Settings.

#include "TrackComponent.h"

#include "Components/InstancedStaticMeshComponent.h"

UTrackComponent::UTrackComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTrackComponent::TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction)
{
	Super::TickComponent(deltaTime, tickType, thisTickFunction);
}

void UTrackComponent::BuildTrack(UInstancedStaticMeshComponent * newMesh)
{
	if(newMesh)
	{
		Mesh = newMesh;
		Mesh->ClearInstances();
		const auto treadLength = GetSplineLength() / TreadCount;

		for (auto i = 0; i < TreadCount; i++)
		{
			Mesh->AddInstance(GetTransformAtDistanceAlongSpline(treadLength * i, ESplineCoordinateSpace::Local));
		}
	}
}

void UTrackComponent::AdjustSpline(const int wheelId, const FVector location, const float wheelRadius)
{
	if(SplinePointBindings.Contains(wheelId))
	{
		auto localWheelPos = GetComponentTransform().InverseTransformPosition(location);
		localWheelPos.Y = 0;

		for (auto pointId : SplinePointBindings[wheelId].PointIds)
		{
			SetLocationAtSplinePoint(pointId, localWheelPos - (wheelRadius + TrackThicknessOffset) * GetUpVectorAtSplinePoint(pointId, ESplineCoordinateSpace::Local), ESplineCoordinateSpace::Local);
		}
	}
}

void UTrackComponent::AnimateTrack(const float deltaTrackOffset)
{
	if(Mesh)
	{
		const auto trackLength = GetSplineLength();
		const auto treadLength = GetSplineLength() / TreadCount;

		const auto lastTrackOffset = TrackOffsetPercentage * trackLength;
		auto trackOffset = FMath::Fmod(lastTrackOffset - deltaTrackOffset, trackLength);
		if (trackOffset < 0)
		{
			trackOffset = trackLength + trackOffset;
		}

		for (auto i = 0; i < TreadCount; i++)
		{
			const auto offset = FMath::Fmod(treadLength * i + trackOffset, trackLength);
			Mesh->UpdateInstanceTransform(i, GetTransformAtDistanceAlongSpline(offset, ESplineCoordinateSpace::Local), false, i == TreadCount - 1, false);
		}

		TrackOffsetPercentage = trackOffset / trackLength;
	}
}

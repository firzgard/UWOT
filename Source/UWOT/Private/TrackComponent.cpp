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
		TreadLength = GetSplineLength() / TreadCount;

		for (auto i = 0; i < TreadCount; i++)
		{
			Mesh->AddInstance(GetTransformAtDistanceAlongSpline(TreadLength * i, ESplineCoordinateSpace::Local));
		}
	}
}

void UTrackComponent::AdjustSpline(const int wheelId, const FVector location, const float wheelRadius)
{
	if(SplinePointBindings.Contains(wheelId))
	{
		auto localWheelPos = GetComponentTransform().InverseTransformPosition(location);
		localWheelPos.Y = 0;

		UE_LOG(LogTemp, Warning, TEXT("Your message, %s"), *localWheelPos.ToString());

		for (auto pointId : SplinePointBindings[wheelId].PointIds)
		{
			SetLocationAtSplinePoint(pointId, localWheelPos - (wheelRadius + TrackThicknessOffset) * GetUpVectorAtSplinePoint(pointId, ESplineCoordinateSpace::World), ESplineCoordinateSpace::Local);
		}
	}
}

void UTrackComponent::AnimateTrack(const float deltaTrackOffset)
{
	if(Mesh)
	{
		TrackOffset = FMath::Fmod(TrackOffset - deltaTrackOffset, GetSplineLength());
		if (TrackOffset < 0)
		{
			TrackOffset = GetSplineLength() + TrackOffset;
		}

		for (auto i = 0; i < TreadCount; i++)
		{
			const auto offset = FMath::Fmod(TreadLength * i + TrackOffset, GetSplineLength());
			Mesh->UpdateInstanceTransform(i, GetTransformAtDistanceAlongSpline(offset, ESplineCoordinateSpace::Local), false, i == TreadCount - 1, false);
		}
	}
}

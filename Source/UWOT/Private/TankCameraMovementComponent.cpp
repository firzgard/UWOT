// Fill out your copyright notice in the Description page of Project Settings.

#include "TankCameraMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/SpringArmComponent.h"


UTankCameraMovementComponent::UTankCameraMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	FirstPersonZoomLevels.Add(1);
	ThirdPersonZoomSteps.Add(1000);
}


void UTankCameraMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(FirstPersonZoomLevels.Num(), TEXT("%s: First person zoom level array is empty!!"), *GetOwner()->GetName());
	checkf(ThirdPersonZoomSteps.Num(), TEXT("%s: Third person zoom step array is empty!!"), *GetOwner()->GetName());
	checkf(ThirdPersonZoomSteps.Num() - 1 > DefaultThirdPersonZoomStepIndex && DefaultThirdPersonZoomStepIndex > 0, TEXT("%s: Invalid default third person zoom step index"), *GetOwner()->GetName());
}

void UTankCameraMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	static const auto TARGET_FIRST_PERSON_ZOOM_TOLERANCE = .1f;
	static const auto TARGET_THIRD_PERSON_ZOOM_TOLERANCE = .5f;

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bBlueprintInitialized)
	{
		if (bUpdateFirstPersonZoom)
		{
			FirstPersonCamera->FieldOfView = BaseFirstPersonFOV / FMath::FInterpTo(BaseFirstPersonFOV / FirstPersonCamera->FieldOfView, TargetFirstPersonZoomLevel, DeltaTime, ZoomSpeed);
			if (FMath::IsNearlyEqual(TargetFirstPersonZoomLevel, BaseFirstPersonFOV / FirstPersonCamera->FieldOfView, TARGET_FIRST_PERSON_ZOOM_TOLERANCE))
			{
				FirstPersonCamera->FieldOfView = BaseFirstPersonFOV / TargetFirstPersonZoomLevel;
				bUpdateFirstPersonZoom = false;
			}
		}

		if (bUpdateThirdPersonZoom)
		{
			ThirdPersonSpringArm->TargetArmLength = FMath::FInterpTo(ThirdPersonSpringArm->TargetArmLength, TargetThirdPersonZoomStep, DeltaTime, ZoomSpeed);
			if (FMath::IsNearlyEqual(TargetThirdPersonZoomStep, ThirdPersonSpringArm->TargetArmLength, TARGET_THIRD_PERSON_ZOOM_TOLERANCE))
			{
				ThirdPersonSpringArm->TargetArmLength = TargetThirdPersonZoomStep;
				bUpdateThirdPersonZoom = false;
			}
		}
	}
}


void UTankCameraMovementComponent::Init(UCameraComponent* newFirstPersonCamera
	, USpringArmComponent* newFirstPersonSpringArm
	, USpringArmComponent* newThirdPersonSpringArm)
{
	checkf(newFirstPersonCamera, TEXT("%s: Failed to init CameraMovementComponent. FirstPersonCamera == nullptr"), *GetOwner()->GetName());
	checkf(newFirstPersonSpringArm, TEXT("%s: Failed to init CameraMovementComponent. FirstPersonSpringArm == nullptr"), *GetOwner()->GetName());
	checkf(newThirdPersonSpringArm, TEXT("%s: Failed to init CameraMovementComponent. ThirdPersonSpringArm == nullptr"), *GetOwner()->GetName());

	FirstPersonCamera = newFirstPersonCamera;
	FirstPersonSpringArm = newFirstPersonSpringArm;
	ThirdPersonSpringArm = newThirdPersonSpringArm;

	BaseFirstPersonFOV = FirstPersonCamera->FieldOfView;
	TargetFirstPersonZoomLevelIndex = 0;
	TargetFirstPersonZoomLevel = FirstPersonZoomLevels[TargetFirstPersonZoomLevelIndex];

	TargetThirdPersonZoomStepIndex = DefaultThirdPersonZoomStepIndex;
	ThirdPersonSpringArm->TargetArmLength = TargetThirdPersonZoomStep = ThirdPersonZoomSteps[TargetThirdPersonZoomStepIndex];

	bUpdateFirstPersonZoom = bUpdateThirdPersonZoom = false;
	bBlueprintInitialized = true;
}

void UTankCameraMovementComponent::SetFirstPersonZoomLevel(int levelIndex)
{
	levelIndex = FMath::Clamp(levelIndex, 0, FirstPersonZoomLevels.Num() - 1);
	if (levelIndex != TargetFirstPersonZoomLevelIndex)
	{
		bUpdateFirstPersonZoom = true;
		TargetFirstPersonZoomLevelIndex = levelIndex;
		TargetFirstPersonZoomLevel = FirstPersonZoomLevels[levelIndex];
	}
}

void UTankCameraMovementComponent::SetThirdPersonZoomStep(int stepIndex)
{
	stepIndex = FMath::ClampAngle(stepIndex, 0, ThirdPersonZoomSteps.Num() - 1);
	if (stepIndex != TargetThirdPersonZoomStepIndex)
	{
		bUpdateThirdPersonZoom = true;
		TargetThirdPersonZoomStepIndex = stepIndex;
		TargetThirdPersonZoomStep = ThirdPersonZoomSteps[stepIndex];
	}
}

void UTankCameraMovementComponent::RotateCameraYaw(float deltaRotationDeg)
{
	if (bBlueprintInitialized)
	{
		const auto deltaRotator = FRotator(0, deltaRotationDeg, 0);
		FirstPersonSpringArm->AddLocalRotation(deltaRotator);
		ThirdPersonSpringArm->AddLocalRotation(deltaRotator);
	}
}

void UTankCameraMovementComponent::RotateCameraPitch(float deltaRotationDeg)
{
	if (bBlueprintInitialized)
	{
		auto targetRotation = FirstPersonSpringArm->RelativeRotation;
		targetRotation.Roll = 0;
		targetRotation.Pitch = FMath::Clamp(targetRotation.Pitch + deltaRotationDeg, CameraPitchMinDeg, CameraPitchMaxDeg);
		FirstPersonSpringArm->SetRelativeRotation(targetRotation);
		ThirdPersonSpringArm->SetRelativeRotation(targetRotation);
	}
}

FVector UTankCameraMovementComponent::GetCameraForwardVector() const
{
	return bBlueprintInitialized ? FirstPersonCamera->GetForwardVector() : FVector::ForwardVector;
}

FVector UTankCameraMovementComponent::GetCameraRightVector() const
{
	return bBlueprintInitialized ? FirstPersonCamera->GetRightVector() : FVector::ForwardVector;
}

FVector UTankCameraMovementComponent::GetCameraUpVector() const
{
	return bBlueprintInitialized ? FirstPersonCamera->GetUpVector() : FVector::ForwardVector;
}


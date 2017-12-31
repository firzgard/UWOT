// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TankCameraMovementComponent.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS( ClassGroup=(UWOT), meta=(BlueprintSpawnableComponent) )
class UWOT_API UTankCameraMovementComponent : public UActorComponent
{
GENERATED_BODY()

private:
	UCameraComponent * FirstPersonCamera = nullptr;
	USpringArmComponent * FirstPersonSpringArm = nullptr;
	USpringArmComponent * ThirdPersonSpringArm = nullptr;

	float BaseFirstPersonFOV = 0;
	bool bUpdateFirstPersonZoom = false;
	bool bUpdateThirdPersonZoom = false;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Components|Zoom Properties")
		float TargetFirstPersonZoomLevel = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Components|Zoom Properties")
		float TargetThirdPersonZoomStep = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Components|Zoom Properties")
		int TargetFirstPersonZoomLevelIndex = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Components|Zoom Properties")
		int TargetThirdPersonZoomStepIndex = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Zoom Properties")
		TArray<float> FirstPersonZoomLevels;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Zoom Properties")
		TArray<float> ThirdPersonZoomSteps;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Zoom Properties")
		int DefaultThirdPersonZoomStepIndex = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Zoom Properties")
		float ZoomSpeed = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Camera Rotation Properties")
		float CameraPitchMaxDeg = 30;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Camera Rotation Properties")
		float CameraPitchMinDeg = -45;

protected:
	virtual void BeginPlay() override;

public:
	UTankCameraMovementComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FVector GetCameraForwardVector() const;
	FVector GetCameraRightVector() const;
	FVector GetCameraUpVector() const;

	UFUNCTION(BlueprintCallable)
		void Init(UCameraComponent * newFirstPersonCamera, USpringArmComponent * newFirstPersonSpringArm, USpringArmComponent * newThirdPersonSpringArm);
	UFUNCTION(BlueprintCallable)
		void SetFirstPersonZoomLevel(int levelIndex);
	UFUNCTION(BlueprintCallable)
		void SetThirdPersonZoomStep(int stepIndex);
	UFUNCTION(BlueprintCallable)
		void RotateCameraYaw(float deltaRotationDeg);
	UFUNCTION(BlueprintCallable)
		void RotateCameraPitch(float deltaRotationDeg);
};

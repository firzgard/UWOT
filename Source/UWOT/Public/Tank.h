// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Tank.generated.h"

class AVehicleImpactEffect;
class UCamouflageComponent;
class UVehicleDustType;
class UTankCameraMovementComponent;
class UTankMainWeaponComponent;
class UTankMovementComponent;

class UAudioComponent;
class UCameraShake;
class UParticleSystemComponent;
class USoundCue;

UCLASS()
class UWOT_API ATank : public APawn
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient)
		TArray<UParticleSystemComponent *> DustParticleComponents;

	bool bHighlighting = false;
	bool bSkidding = false;
	bool bOnAir = false;
	bool bFullThrottleSfxPlayed = false;
	bool bTrackRollingSfxPlaying = false;
	float SkidStartTime = 0;

protected:
	UPROPERTY(VisibleAnywhere, Category = Components)
		UAudioComponent* EngineAudioComponent;
	UPROPERTY(VisibleAnywhere, Category = Components)
		UAudioComponent* SkidAudioComponent;
	UPROPERTY(VisibleAnywhere, Category = Components)
		UAudioComponent * TurretRotateAudioComponent;
	UPROPERTY(VisibleAnywhere, Category = Components)
		UAudioComponent * TrackRollingAudioComponent;
	UPROPERTY(VisibleAnywhere, Category = Components)
		UAudioComponent * FullThrottleRotateAudioComponent;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UVehicleDustType* DustFX;
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		TSubclassOf<AVehicleImpactEffect> ImpactFXs;

	/** camera shake on impact */
	UPROPERTY(Category = Effects, EditDefaultsOnly)
		TSubclassOf<UCameraShake> ImpactCameraShake;

	/** The minimum amount of normal force that must be applied to the chassis to spawn an Impact Effect */
	UPROPERTY(EditAnywhere, Category = Effects)
		float ImpactEffectNormalForceThreshold = 100000;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UParticleSystem* DeathVFX;
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		USoundCue * DeathSFX;
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		USoundCue * EngineSFX;
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		USoundCue * EngineIgnitionSFX;
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		USoundCue * EngineFullThrottleSFX;
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		USoundCue * TrackRollingSFX;
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		USoundCue * TurretRotateSFX;
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		float EngineIgnitionDuration = 1;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
		USoundCue * LandingSFX;
	/** The amount of spring compression required during landing to play sound */
	UPROPERTY(Category = Effects, EditDefaultsOnly)
		float SpringCompressionLandingThreshold = 250000;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
		USoundCue * SkiddingSFX;
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		USoundCue * SkidStoppingSFX;
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		float SkidFadeoutTime = 1;
	/** Skid effects cannot play if velocity is lower than this */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		float SkidSFXSpeedThreshold = 10;
	/** Skit effects will play if absolute value of tire longitudinal slip is more than this. */
	UPROPERTY(EditDefaultsOnly, Category = Effects, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
		float SkidSFXLongSlipThreshold = 0.8;
	/** Skid effects will play if absolute value of tire lateral slip is more than this. */
	UPROPERTY(EditDefaultsOnly, Category = Effects, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
		float SkidSFXLateralSlipThreshold = 0.8;
	/** If skidding time is shorter than this value, skid stop sfx won't be played */
	UPROPERTY(Category = Effects, EditDefaultsOnly)
		float SkidStoppingSFXMinSkidDuration = 0.5;

	/** track effects cannot play if velocity is lower than this */
	UPROPERTY(Category = Effects, EditDefaultsOnly)
		float TrackRollingSFXSpeedThreshold = 15;
	UPROPERTY(Category = Effects, EditDefaultsOnly)
		float TrackRollingSfxFadeoutTime = 1;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
		USkeletalMeshComponent * ChassisMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UTankCameraMovementComponent * CameraMovementComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UTankMainWeaponComponent * MainWeaponComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UTankMovementComponent * MovementComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UCamouflageComponent * CamouflageComponent;

	/** Identifies if pawn is in its dying state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Health, ReplicatedUsing = OnReplicate_Dying)
		bool bIsDying = false;

protected:
	// Begin Actor overrides
	void PostInitializeComponents() override;
	void BeginPlay() override;
	void Tick(float deltaTime) override;
	void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalForce, const FHitResult& Hit) override;
	void FellOutOfWorld(const UDamageType& dmgType) override;
	// End Actor overrides

	// Begin Pawn overrides
	void DisplayDebug(class UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
	void SetupPlayerInputComponent(class UInputComponent* playerInputComponent) override;
	float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	void TornOff() override;
	void UnPossessed() override;
	// End Pawn overrides

	/** Returns True if the pawn can die in the current state */
	virtual bool CanDie() const;

	/** Kills pawn. [Server/authority only] */
	virtual void Die();

	/** Event on death [Server/Client] */
	virtual void OnDeath();

	/** Replicating death on client */
	UFUNCTION()
		void OnReplicate_Dying();

	/** Update effects under wheels */
	void UpdateWheelEffects();

public:
	ATank(const FObjectInitializer& objectInitializer);

	UFUNCTION(BlueprintCallable)
		void SetHighlight(bool bHighlight);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "SetHighlight"))
		void ReceiveSetHighlight(bool bHighlight);

	UFUNCTION(BlueprintCallable)
		float GetHullAlignment() const;
	UFUNCTION(BlueprintCallable)
		float GetTurretAlignment() const;
};

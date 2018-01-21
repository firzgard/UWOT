// Fill out your copyright notice in the Description page of Project Settings.

#include "Tank.h"

#include "CamouflageComponent.h"
#include "VehicleDustType.h"
#include "TankCameraMovementComponent.h"
#include "TankMainWeaponComponent.h"
#include "TankMovementComponent.h"
#include "VehicleEngineSoundNode.h"
#include "VehicleImpactEffect.h"

#include "AudioThread.h"
#include "Components/AudioComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DisplayDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UnrealNetwork.h"

// Sets default values
ATank::ATank(const FObjectInitializer& objectInitializer) : Super(objectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	ChassisMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Chassis");
	ChassisMesh->SetCollisionProfileName(UCollisionProfile::Vehicle_ProfileName);
	ChassisMesh->BodyInstance.bSimulatePhysics = true;
	ChassisMesh->BodyInstance.bNotifyRigidBodyCollision = true;
	ChassisMesh->BodyInstance.bUseCCD = true;
	ChassisMesh->bBlendPhysics = true;
	ChassisMesh->bGenerateOverlapEvents = true;
	ChassisMesh->SetCanEverAffectNavigation(true);
	RootComponent = ChassisMesh;
	
	CameraMovementComponent = CreateDefaultSubobject<UTankCameraMovementComponent>(FName("TankCameraMovementComponent"));
	MainWeaponComponent = CreateDefaultSubobject<UTankMainWeaponComponent>(FName("TankMainWeaponComponent"));
	CamouflageComponent = CreateDefaultSubobject<UCamouflageComponent>(FName("TankCamouflageComponent"));

	MovementComponent = CreateDefaultSubobject<UTankMovementComponent>(FName("TankMovementComponent"));
	MovementComponent->SetIsReplicated(true);
	MovementComponent->UpdatedComponent = ChassisMesh;

	EngineAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudio"));
	EngineAudioComponent->SetupAttachment(ChassisMesh);

	SkidAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SkidAudio"));
	SkidAudioComponent->bAutoActivate = false;
	SkidAudioComponent->SetupAttachment(ChassisMesh);

	TrackRollingAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("TrackRollingAudio"));
	TrackRollingAudioComponent->bAutoActivate = false;
	TrackRollingAudioComponent->SetupAttachment(ChassisMesh);

	FullThrottleRotateAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("FullThrottleRotateAudio"));
	FullThrottleRotateAudioComponent->bAutoActivate = false;
	FullThrottleRotateAudioComponent->SetupAttachment(ChassisMesh);

	TurretRotateAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("TurretRotateAudio"));
	TurretRotateAudioComponent->bAutoActivate = false;
	TurretRotateAudioComponent->SetupAttachment(ChassisMesh);
	MainWeaponComponent->SetTurretRotateAudioComponent(TurretRotateAudioComponent);
}

void ATank::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (EngineAudioComponent)
	{
		EngineAudioComponent->SetSound(EngineSFX);
		EngineAudioComponent->Play();
	}

	if (SkidAudioComponent)
	{
		SkidAudioComponent->SetSound(SkiddingSFX);
		SkidAudioComponent->Stop();
	}

	if(TurretRotateAudioComponent)
	{
		TurretRotateAudioComponent->SetSound(TurretRotateSFX);
		TurretRotateAudioComponent->Stop();
	}

	if(FullThrottleRotateAudioComponent)
	{
		FullThrottleRotateAudioComponent->SetSound(EngineFullThrottleSFX);
		FullThrottleRotateAudioComponent->Stop();
	}

	if(TrackRollingAudioComponent)
	{
		TrackRollingAudioComponent->SetSound(TrackRollingSFX);
		TrackRollingAudioComponent->Stop();
	}
}

void ATank::BeginPlay()
{
	Super::BeginPlay();

	DustParticleComponents.SetNum(MovementComponent->Wheels.Num());

	// Ignite engine
	if (EngineIgnitionSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EngineIgnitionSFX, GetActorLocation(), GetActorRotation(), 1, 1, 0, nullptr, nullptr, this);

		if(const auto player = Cast<APlayerController>(Controller))
		{
			DisableInput(player);
			
			FTimerHandle timerHandle;
			GetWorld()->GetTimerManager().SetTimer(timerHandle, [this, player]
			{
				EnableInput(player);
			}, EngineIgnitionDuration, false);
		}
	}
}

void ATank::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	UpdateWheelEffects();

	// Play full-throttle sfx
	const auto currentThrottle = MovementComponent->GetThrottleInput();
	UE_LOG(LogTemp, Warning, TEXT("Your message %f"), currentThrottle);
	if(currentThrottle == 0)
	{
		bFullThrottleSfxPlayed = false;
		FullThrottleRotateAudioComponent->FadeOut(1, 0);
	}

	if(!bFullThrottleSfxPlayed && currentThrottle > 0)
	{
		bFullThrottleSfxPlayed = true;
		FullThrottleRotateAudioComponent->Play();
	}

	if (Controller)
	{
		const auto playerId = Controller->GetUniqueID();

		UVehicleEngineSoundNode::FVehicleDesiredRPM desiredRpm;
		desiredRpm.DesiredRPM = FMath::Abs(MovementComponent->GetEngineRotationSpeed());
		desiredRpm.TimeStamp = GetWorld()->GetTimeSeconds();

		// TODO Use which one again??
		{
			UVehicleEngineSoundNode::SetDesiredRPM(playerId, desiredRpm);
			EngineAudioComponent->SetFloatParameter(FName("RPM"), desiredRpm.DesiredRPM);
		}
	}
}

void ATank::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved,
	FVector HitLocation, FVector HitNormal, FVector NormalForce, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalForce, Hit);

	// Spawn impact fx
	if (ImpactFXs && NormalForce.SizeSquared() > FMath::Square(ImpactEffectNormalForceThreshold))
	{
		const FTransform spawnTransform(HitNormal.Rotation(), HitLocation);
		auto effectActor = GetWorld()->SpawnActorDeferred<AVehicleImpactEffect>(ImpactFXs, spawnTransform);
		if (effectActor)
		{
			effectActor->HitSurface = Hit;
			effectActor->HitForce = NormalForce;
			// TODO: Magic number here
			effectActor->bWheelLand = FVector::DotProduct(HitNormal, GetActorUpVector()) > 0.8f;

			UGameplayStatics::FinishSpawningActor(effectActor, spawnTransform);
		}
	}

	// Shake cam
	if (ImpactCameraShake)
	{
		auto player = Cast<APlayerController>(Controller);
		if (player && player->IsLocalController())
		{
			player->ClientPlayCameraShake(ImpactCameraShake, 1);
		}
	}
}

void ATank::FellOutOfWorld(const UDamageType & dmgType)
{
	Die();
}


void ATank::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	static FName NAME_Vehicle = FName(TEXT("Vehicle"));

	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);

	if (DebugDisplay.IsDisplayOn(NAME_Vehicle))
	{
		MovementComponent->DrawDebug(Canvas, YL, YPos);
	}
}

void ATank::SetupPlayerInputComponent(UInputComponent* playerInputComponent)
{
	Super::SetupPlayerInputComponent(playerInputComponent);
}

float ATank::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// Die();

	return Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

void ATank::TornOff()
{
	Super::TornOff();

	SetLifeSpan(1);
}

void ATank::UnPossessed()
{
	if (Controller)
	{
		const auto playerId = Controller->GetUniqueID();
		UVehicleEngineSoundNode::RemoveDesiredRPM(playerId);
	}

	// Super clears controller, so do the behavior first
	Super::UnPossessed();
}



#pragma region Networking

	void ATank::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);

		DOREPLIFETIME(ATank, bIsDying);
	}

	void ATank::OnReplicate_Dying()
	{
		if (bIsDying)
		{
			OnDeath();
		}
	}

#pragma endregion Networking


bool ATank::CanDie() const
{
	if (bIsDying										// already dying
		|| IsPendingKill()								// already destroyed
		|| Role != ROLE_Authority)						// not authority
	{
		return false;
	}

	return true;
}

void ATank::Die()
{
	if (CanDie())
	{
		OnDeath();
	}
}

void ATank::OnDeath()
{
	bReplicateMovement = false;
	bTearOff = true;
	bIsDying = true;

	DetachFromControllerPendingDestroy();

	// hide and disable
	TurnOff();
	SetActorHiddenInGame(true);

	if (EngineAudioComponent)
	{
		EngineAudioComponent->Stop();
	}

	if (SkidAudioComponent)
	{
		SkidAudioComponent->Stop();
	}

	if (DeathVFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, DeathVFX, GetActorLocation(), GetActorRotation());
	}

	if (DeathSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSFX, GetActorLocation());
	}

	// Finite lifespan so that auto destroying can kick in
	SetLifeSpan(0.2f);
}


void ATank::UpdateWheelEffects()
{
	const auto bOnAirLastFrame = bOnAir;
	bOnAir = true;
	const auto currentSpeed = MovementComponent->GetForwardSpeed();

	// Spawn dust vfx
	if (DustFX && !bIsDying && MovementComponent->Wheels.Num() > 0)
	{
		for (auto i = 0; i < DustParticleComponents.Num(); i++)
		{
			const auto contactMat = MovementComponent->Wheels[i]->GetContactSurfaceMaterial();
			const auto dustParticle = DustFX->GetDustFX(contactMat, currentSpeed);

			// Considered on air if all wheels doesnt come into contact with surface
			if (contactMat != nullptr)
			{
				bOnAir = false;
			}

			auto dustParticleComponent = DustParticleComponents[i];
			const auto bIsActive = dustParticleComponent && !dustParticleComponent->bWasDeactivated && !dustParticleComponent->bWasCompleted;
			const auto currentDustParticle = dustParticleComponent ? dustParticleComponent->Template : nullptr;

			if (dustParticle && (currentDustParticle != dustParticle || !bIsActive))
			{
				// Replace old particle system component before applying new particle
				if (dustParticleComponent == nullptr || !dustParticleComponent->bWasDeactivated)
				{
					if (dustParticleComponent != nullptr)
					{
						dustParticleComponent->SetActive(false);
						dustParticleComponent->bAutoDestroy = true;
					}

					DustParticleComponents[i] = dustParticleComponent = NewObject<UParticleSystemComponent>(this);
					dustParticleComponent->bAutoActivate = true;
					dustParticleComponent->bAutoDestroy = false;
					dustParticleComponent->RegisterComponentWithWorld(GetWorld());
					dustParticleComponent->AttachToComponent(ChassisMesh, FAttachmentTransformRules::KeepRelativeTransform, MovementComponent->WheelSetups[i].BoneName);
				}

				dustParticleComponent->SetTemplate(dustParticle);
				dustParticleComponent->ActivateSystem();
			}
			else if (!dustParticle && bIsActive)
			{
				dustParticleComponent->SetActive(false);
			}
		}
	}

	if (bOnAirLastFrame && !bOnAir && LandingSFX)
	{
		if (MovementComponent->GetMaxSpringForce() > SpringCompressionLandingThreshold)
		{
			UGameplayStatics::PlaySoundAtLocation(this, LandingSFX, GetActorLocation());
		}
	}

	// Add skidding sfx
	if (SkidAudioComponent)
	{
		const auto bVehicleStopped = currentSpeed < SkidSFXSpeedThreshold;
		const auto bTireSlipping = MovementComponent->CheckSlipThreshold(SkidSFXLongSlipThreshold, SkidSFXLateralSlipThreshold);
		const auto bShallSkid = !bOnAir && !bVehicleStopped && bTireSlipping;

		const auto currentTime = GetWorld()->GetTimeSeconds();
		
		if (bShallSkid && !bSkidding)
		{
			bSkidding = true;
			SkidAudioComponent->Play();
			SkidStartTime = currentTime;
		}
		
		else if (!bShallSkid && bSkidding)
		{
			bSkidding = false;
			SkidAudioComponent->FadeOut(SkidFadeoutTime, 0);
			if (currentTime - SkidStartTime > SkidStoppingSFXMinSkidDuration)
			{
				UGameplayStatics::PlaySoundAtLocation(this, SkidStoppingSFX, GetActorLocation());
			}
		}
	}

	// Add track rolling sfx
	if(TrackRollingAudioComponent)
	{
		// TODO: Hard code the name here
		TrackRollingAudioComponent->SetFloatParameter("Speed", currentSpeed);

		if(currentSpeed < TrackRollingSFXSpeedThreshold)
		{
			if(bTrackRollingSfxPlaying)
			{
				TrackRollingAudioComponent->FadeOut(TrackRollingSfxFadeoutTime, 0);
				bTrackRollingSfxPlaying = false;
			}
		} 
		else
		{
			if(!bTrackRollingSfxPlaying)
			{
				TrackRollingAudioComponent->Play();
				bTrackRollingSfxPlaying = true;
			}
		}
	}
}

float ATank::GetHullAlignment() const
{
	const auto hullYawVector = FVector::VectorPlaneProject(GetActorForwardVector(), CameraMovementComponent->GetCameraUpVector()).GetSafeNormal();
	auto angle = FMath::RadiansToDegrees(FMath::Acos(hullYawVector | CameraMovementComponent->GetCameraForwardVector()));
	return (hullYawVector | CameraMovementComponent->GetCameraRightVector()) > 0 ? angle : -angle;
}

float ATank::GetTurretAlignment() const
{
	const auto turretYawVector = FVector::VectorPlaneProject(MainWeaponComponent->GetTurretForwardVector(), CameraMovementComponent->GetCameraUpVector()).GetSafeNormal();
	auto angle = FMath::RadiansToDegrees(FMath::Acos(turretYawVector | CameraMovementComponent->GetCameraForwardVector()));
	return (turretYawVector | CameraMovementComponent->GetCameraRightVector()) > 0 ? angle : -angle;
}
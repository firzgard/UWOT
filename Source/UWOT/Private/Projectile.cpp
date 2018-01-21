// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"

// Sets default values
AProjectile::AProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("CollisionMesh"));

	SetRootComponent(CollisionMesh);
	CollisionMesh->SetNotifyRigidBodyCollision(true);
	CollisionMesh->BodyInstance.bUseCCD = true;
	// Without physics, projectile will penetrate target's mesh and push target back upon hitting.
	CollisionMesh->SetSimulatePhysics(true);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(FName("MovementComponent"));

	ExplosionForceComponent = CreateDefaultSubobject<URadialForceComponent>(FName("ExplosionForceComponent"));
	ExplosionForceComponent->SetupAttachment(CollisionMesh);
}

void AProjectile::Fire(AActor * owner)
{
	SetOwner(owner);
	SetLifeSpan(LifeTimeSec);

	// Add firing effect
	if(MuzzleBlastVfx)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleBlastVfx, CollisionMesh->GetComponentTransform());
	}
	if(MuzzleBlastSfx)
	{
		UGameplayStatics::PlaySoundAtLocation(this, MuzzleBlastSfx, CollisionMesh->GetComponentLocation());
	}
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AProjectile::Tick(float deltaTime)
{
	Super::Tick(deltaTime);
}

void AProjectile::NotifyHit(UPrimitiveComponent* MyComp
	, AActor* Other
	, UPrimitiveComponent* OtherComp
	, bool bSelfMoved
	, FVector HitLocation
	, FVector HitNormal
	, FVector NormalImpulse
	, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, NormalImpulse, HitNormal, Hit);
	
	if (ensureMsgf(ImpactBlastVfx, TEXT("No ImpactBlast particle system specified.")))
	{
		SetRootComponent(ExplosionForceComponent);
		ExplosionForceComponent->FireImpulse();

		// Spawn fxs
		if(ImpactBlastSfx)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactBlastVfx, CollisionMesh->GetComponentTransform());
		}
		if(ImpactBlastVfx)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ImpactBlastSfx, CollisionMesh->GetComponentLocation());
		}

		// Apply dmg
		UGameplayStatics::ApplyRadialDamageWithFalloff(this, FullDamage, 0
			, GetActorLocation()
			, FullDamageRadius
			, ExplosionForceComponent->Radius
			, DamageFalloff
			, UDamageType::StaticClass()
			, TArray<AActor *>()
			, GetOwner()
			, GetOwner() ? GetOwner()->GetInstigatorController() : nullptr
			, ECC_WorldStatic);

		CollisionMesh->DestroyComponent();
		// Somehow destroying this projectile now will not trigger OnHit on the other Actor/Component
		// Delay destroy to next frame
		GetWorldTimerManager().SetTimerForNextTick([this]()
		{
			Destroy();
		});
	}
}

float AProjectile::GetSpeed() const { return ProjectileMovementComponent->InitialSpeed; }

UParticleSystem* AProjectile::GetMuzzleBlastVfx() const
{
	return MuzzleBlastVfx;
}

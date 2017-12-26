// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile.h"

#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "PhysicsEngine/RadialForceActor.h"
#include "TimerManager.h"
#include "Engine/World.h"

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
	ExplosionForceComponent->AttachToComponent(CollisionMesh, FAttachmentTransformRules::KeepRelativeTransform);
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
		ExplosionForceComponent->FireImpulse();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactBlastVfx, CollisionMesh->GetComponentLocation());
		FTimerHandle UnusedHandle;

		SetActorEnableCollision(false);
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

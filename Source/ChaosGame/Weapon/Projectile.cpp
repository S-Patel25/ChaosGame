// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "ChaosGame/Character/ChaosCharacter.h"
#include "ChaosGame/ChaosGame.h"
#include "NiagaraComponent.h"
#include "NiagaraSystemInstanceController.h"
#include "NiagaraFunctionLibrary.h"

AProjectile::AProjectile()
{

	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	collisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	SetRootComponent(collisionBox);

	collisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic); //since this will be flying through world, set as world dynamic
	collisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	collisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	collisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	collisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic , ECollisionResponse::ECR_Block);
	collisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block); //so we dont trace on the pawn, only mesh

}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (tracer)
	{
		tracerComponent = UGameplayStatics::SpawnEmitterAttached( //spawns the emitter, will follow trail of projectile
			tracer,
			collisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}

	if (HasAuthority())
	{
		collisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit); //bind delegate to our function
	}
}

void AProjectile::startDestroyTimer()
{
	GetWorldTimerManager().SetTimer( //setting timer to when rocket gets destroyed
		destroyTimer,
		this,
		&AProjectile::destroyTimerFinished,
		destroyTime
	);
}

void AProjectile::destroyTimerFinished()
{
	Destroy(); 
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

	Destroy(); //so it doesn't stick (overrided function is already replicated, so we take advantage of that)
}

void AProjectile::spawnTrailSystem()
{
	if (trailSystem)
	{
		trailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			trailSystem,
			GetRootComponent(), //spawns niagara emitter
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
}

void AProjectile::explodeDamage()
{
	APawn* firingPawn = GetInstigator();

	if (firingPawn && HasAuthority()) //only server deals with damage
	{
		AController* firingController = firingPawn->GetController();
		if (firingController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, //world contect object
				Damage, //base damage
				10.f, //min damage
				GetActorLocation(), //origin
				DamageInnerRaidus, //inner radius
				DamageOuterRadius, //outer raidus
				1.f, //damage falloff
				UDamageType::StaticClass(), //damagetype class
				TArray<AActor*>(), //ignore actors
				this, //damage causer
				firingController //instigatorcontroller
			); //use rocket projectile for radial explosive damagge
		}
	}

}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::Destroyed()
{
	Super::Destroyed();

	if (impactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), impactParticles, GetActorTransform()); //spawn impact sound
	}

	if (impactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, impactSound, GetActorLocation());
	}
}


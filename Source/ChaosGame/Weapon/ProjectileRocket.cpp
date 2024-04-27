// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystemInstanceController.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "RocketMovementComponent.h"


AProjectileRocket::AProjectileRocket()
{
	projectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	projectileMesh->SetupAttachment(RootComponent);
	projectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); //since just a tracer

	rocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	rocketMovementComponent->bRotationFollowsVelocity = true;
	rocketMovementComponent->SetIsReplicated(true);
}


void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		collisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit); //bind delegate to our function
	}

	spawnTrailSystem();

	if (projectileLoop && loopingSoundAttenuation)
	{
		projectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
			projectileLoop,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.f,
			1.f,
			0.f,
			loopingSoundAttenuation,
			(USoundConcurrency*)nullptr, //c style cast
			false
		);
	}
}


void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner())
	{
		return;
	}

	explodeDamage();
	

	startDestroyTimer();

	if (impactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), impactParticles, GetActorTransform()); //spawn impact sound
	}

	if (impactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, impactSound, GetActorLocation());
	}

	if (projectileMesh)
	{
		projectileMesh->SetVisibility(false);
	}

	if (collisionBox)
	{
		collisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision); //so weird things dont happen once it hits somehting
	}

	if (trailSystemComponent && trailSystemComponent->GetSystemInstanceController())
	{
		trailSystemComponent->GetSystemInstanceController()->Deactivate();
	}
	
	if (projectileLoopComponent && projectileLoopComponent->IsPlaying())
	{
		projectileLoopComponent->Stop(); //stop playing sound once hit
	}


	//Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);

}

void AProjectileRocket::Destroyed()
{

}
// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


AProjectileGrenade::AProjectileGrenade()
{
	projectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade Mesh"));
	projectileMesh->SetupAttachment(RootComponent);
	projectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); //since just a tracer

	projectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	projectileMovementComponent->bRotationFollowsVelocity = true; //fallof and rotation remains true
	projectileMovementComponent->SetIsReplicated(true);
	projectileMovementComponent->bShouldBounce = true; //allow for grenade to bounce
}

void AProjectileGrenade::Destroyed()
{
	explodeDamage(); //for grenade and rocket
	Super::Destroyed();
}

void AProjectileGrenade::BeginPlay()
{
	AActor::BeginPlay(); //dont need projectile beginplay

	projectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);

	spawnTrailSystem();
	startDestroyTimer();

}

void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if (grenadeBounceSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			grenadeBounceSound,
			GetActorLocation()
		);
	}

}

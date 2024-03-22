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


AProjectileRocket::AProjectileRocket()
{
	rocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	rocketMesh->SetupAttachment(RootComponent);
	rocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); //since just a tracer

}


void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		collisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit); //bind delegate to our function
	}

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

void AProjectileRocket::destroyTimerFinished()
{
	Destroy(); //delay when rocket explodes so trail is still there upon impact
}



void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
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
				200.f, //inner radius
				500.f, //outer raidus
				1.f, //damage falloff
				UDamageType::StaticClass(), //damagetype class
				TArray<AActor*>(), //ignore actors
				this, //damage causer
				firingController //instigatorcontroller
			); //use rocket projectile for radial explosive damagge
		}
	}

	GetWorldTimerManager().SetTimer( //setting timer to when rocket gets destroyed
		destroyTimer,
		this,
		&AProjectileRocket::destroyTimerFinished,
		destroyTime
	);

	if (impactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), impactParticles, GetActorTransform()); //spawn impact sound
	}

	if (impactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, impactSound, GetActorLocation());
	}

	if (rocketMesh)
	{
		rocketMesh->SetVisibility(false);
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
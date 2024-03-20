// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	rocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	rocketMesh->SetupAttachment(RootComponent);
	rocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); //since just a tracer

}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	APawn* firingPawn = GetInstigator(); 

	if (firingPawn)
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


	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);

}

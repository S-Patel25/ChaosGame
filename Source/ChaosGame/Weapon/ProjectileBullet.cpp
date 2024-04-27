// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Character.h"

AProjectileBullet::AProjectileBullet()
{
	projectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	projectileMovementComponent->bRotationFollowsVelocity = true; //fallof and rotation remains true
	projectileMovementComponent->SetIsReplicated(true);
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ACharacter* ownerCharacter = Cast<ACharacter>(GetOwner()); //since ownership eventually leads to character
	if (ownerCharacter)
	{
		AController* ownerController = ownerCharacter->Controller;

		if (ownerController)
		{
			UGameplayStatics::ApplyDamage(OtherActor, Damage, ownerController, this, UDamageType::StaticClass()); //using applydamge function to "take damage"
		}
	}


	

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit); //should go last since parent class destroys projectile, so get everything done first then call super
}

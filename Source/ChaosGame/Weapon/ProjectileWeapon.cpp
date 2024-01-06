// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	APawn* instigatorPawn = Cast<APawn>(GetOwner()); //to satisfy spawn params

	const USkeletalMeshSocket* muzzleFlashSocket = getWeaponMesh()->GetSocketByName(FName("MuzzleFlash")); //getting muzzle flash socket as that's where the projectile will spawn

	if (muzzleFlashSocket)
	{
		FTransform socketTransform = muzzleFlashSocket->GetSocketTransform(getWeaponMesh()); //get transform of muzzle flash socket
		FVector toTarget = HitTarget - socketTransform.GetLocation(); //vector from tip of barrel to the hit target (trace)
		FRotator targetRotation = toTarget.Rotation();


		if (ProjectileClass && instigatorPawn)
		{
			FActorSpawnParameters spawnParams;
			spawnParams.Owner = GetOwner(); //owner is the character
			spawnParams.Instigator = instigatorPawn;

			UWorld* world = GetWorld();

			if (world)
			{
				world->SpawnActor<AProjectile>(
					ProjectileClass,
					socketTransform.GetLocation(),
					targetRotation,
					spawnParams
				); //spawn actor
			}
		}


	}

}

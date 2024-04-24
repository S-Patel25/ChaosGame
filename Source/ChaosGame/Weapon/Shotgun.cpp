// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "ChaosGame/Character/ChaosCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget); //so fire anim and ammo gets updated correctly
	APawn* ownerPawn = Cast<APawn>(GetOwner());
	if (ownerPawn == nullptr) return;

	AController* instigatorController = ownerPawn->GetController();

	const USkeletalMeshSocket* muzzleFlashSocket = getWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (muzzleFlashSocket)
	{
		FTransform socketTransform = muzzleFlashSocket->GetSocketTransform(getWeaponMesh()); //getting all this for line trace
		FVector Start = socketTransform.GetLocation();

		for (uint32 x = 0; x < numPellets; x++) //for loop causes multiple pellets
		{
			FVector End = traceEndWithScatter(Start, HitTarget);
		}
		
	}
}

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

		TMap<AChaosCharacter*, uint32> hitMap;

		for (uint32 x = 0; x < numPellets; x++) //for loop causes multiple pellets
		{
			FHitResult fireHit;
			weaponTraceHit(Start, HitTarget, fireHit);

			AChaosCharacter* chaosCharacter = Cast<AChaosCharacter>(fireHit.GetActor()); //get char
			if (chaosCharacter && HasAuthority() && instigatorController) //moved check so particles can be seen on simulated proxies aswell
			{
				if (hitMap.Contains(chaosCharacter))
				{
					hitMap[chaosCharacter]++; //add to the map so we can track number of hits based on the pellets
				}
				else
				{
					hitMap.Emplace(chaosCharacter, 1);
				}
			}
			if (impactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					impactParticles,
					fireHit.ImpactPoint,
					fireHit.ImpactNormal.Rotation()
				);
			}
			if (hitSound) //adding these as SMG does not have anims and sounds in the pack
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					hitSound,
					fireHit.ImpactPoint,
					.5f,
					FMath::FRandRange(-.5f, .5f)
				);
			}
		}
		for (auto hitPair : hitMap)
		{
			if (instigatorController)
			{
				if (hitPair.Key && HasAuthority() && instigatorController) //moved check so particles can be seen on simulated proxies aswell
				{
					UGameplayStatics::ApplyDamage( //hit scan by line trace
						hitPair.Key, //hitpair.Key is chaosCharacter 
						Damage * hitPair.Value, //num of hits by the pellets
						instigatorController,
						this,
						UDamageType::StaticClass()
					);
				}
			}
		}
	}
}

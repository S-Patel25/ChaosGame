// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "ChaosGame/Character/ChaosCharacter.h"
#include "Kismet/GameplayStatics.h"


void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* ownerPawn = Cast<APawn>(GetOwner());
	if (ownerPawn == nullptr) return;

	AController* instigatorController = ownerPawn->GetController();

	const USkeletalMeshSocket* muzzleFlashSocket = getWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (muzzleFlashSocket && instigatorController)
	{
		FTransform socketTransform = muzzleFlashSocket->GetSocketTransform(getWeaponMesh()); //getting all this for line trace
		FVector Start = socketTransform.GetLocation();
		FVector End = Start + (HitTarget - Start) - 1.25f; //so it properly registers the hit


		FHitResult fireHit;	
		UWorld* world = GetWorld();

		if (world)
		{
			world->LineTraceSingleByChannel(
				fireHit,
				Start,
				End,
				ECollisionChannel::ECC_Visibility
			);

			if (fireHit.bBlockingHit)
			{
				AChaosCharacter* chaosCharacter = Cast<AChaosCharacter>(fireHit.GetActor()); //get char
				if (chaosCharacter)
				{
					if (HasAuthority()) //damage is only applied on server
					{
						UGameplayStatics::ApplyDamage( //hit scan by line trace
							chaosCharacter,
							Damage,
							instigatorController,
							this, 
							UDamageType::StaticClass()
						);
					}
				}
				if (impactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						world,
						impactParticles,
						fireHit.ImpactPoint,
						fireHit.ImpactNormal.Rotation()
					);
				}
			}
		}
	}
}

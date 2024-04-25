// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "ChaosGame/Character/ChaosCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "WeaponTypes.h"


void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* ownerPawn = Cast<APawn>(GetOwner());
	if (ownerPawn == nullptr) return;

	AController* instigatorController = ownerPawn->GetController();

	const USkeletalMeshSocket* muzzleFlashSocket = getWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (muzzleFlashSocket)
	{
		FTransform socketTransform = muzzleFlashSocket->GetSocketTransform(getWeaponMesh()); //getting all this for line trace
		FVector Start = socketTransform.GetLocation();

		FHitResult fireHit;
		weaponTraceHit(Start, HitTarget, fireHit);

		AChaosCharacter* chaosCharacter = Cast<AChaosCharacter>(fireHit.GetActor()); //get char
		if (chaosCharacter && HasAuthority() && instigatorController) //moved check so particles can be seen on simulated proxies aswell
		{
			UGameplayStatics::ApplyDamage( //hit scan by line trace
				chaosCharacter,
				Damage,
				instigatorController,
				this,
				UDamageType::StaticClass()
			);
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

		if (muzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				muzzleFlash,
				socketTransform
			);
		}
		if (fireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				fireSound,
				GetActorLocation()
			);
		}
		if (hitSound) //adding these as SMG does not have anims and sounds in the pack
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				hitSound,
				fireHit.ImpactPoint
			);
		}
	}
}

FVector AHitScanWeapon::traceEndWithScatter(const FVector& traceStart, const FVector& hitTarget)
{
	FVector toTargetNormalized = (hitTarget - traceStart).GetSafeNormal(); //vector from trace start to hit point
	FVector sphereCenter = traceStart + toTargetNormalized * distanceToSphere; //location from trace start outwards based on distance to sphere value
	FVector randVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, sphereRadius);
	FVector endLoc = sphereCenter + randVec; //these vecs are to find a rand point inside the distance sphere for shotgun scatter
	FVector toEndLoc = endLoc - traceStart;

	DrawDebugSphere(GetWorld(), sphereCenter, sphereRadius, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), endLoc, 4.f, 12, FColor::Orange, true);
	DrawDebugLine(GetWorld(), traceStart, FVector(traceStart + toEndLoc * TRACE_LENGTH / toEndLoc.Size()), FColor::Cyan, true);

	return FVector(traceStart + toEndLoc * TRACE_LENGTH / toEndLoc.Size()); //we divide since trace length is very large
}

void AHitScanWeapon::weaponTraceHit(const FVector& traceStart, const FVector& hitTarget, FHitResult& outHit) //can change method type or just have reference to hit result
{
	UWorld* world = GetWorld();
	if (world)
	{
		FVector End = bUseScatter ? traceEndWithScatter(traceStart, hitTarget) : traceStart + (hitTarget - traceStart); //ternary operator based on scatter bool

		world->LineTraceSingleByChannel(
			outHit,
			traceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);
		FVector beamEnd = End;
		if (outHit.bBlockingHit)
		{
			beamEnd = outHit.ImpactPoint;

			if (beamParticles)
			{
				UParticleSystemComponent* beam = UGameplayStatics::SpawnEmitterAtLocation( //spawn particle
					world,
					beamParticles,
					traceStart,
					FRotator::ZeroRotator,
					true
				);
				if (beam)
				{
					beam->SetVectorParameter(FName("Target"), beamEnd); //set endpoint for particle system
				}
			}
		}

	}

}


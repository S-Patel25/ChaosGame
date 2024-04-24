// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSGAME_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:

	virtual void Fire(const FVector& HitTarget) override;

protected:

private:

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* impactParticles; //to show hit

	UPROPERTY(EditAnywhere)
	UParticleSystem* beamParticles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* muzzleFlash;

	UPROPERTY(EditAnywhere)
	USoundCue* fireSound;

	UPROPERTY(EditAnywhere)
	USoundCue* hitSound;
	
};

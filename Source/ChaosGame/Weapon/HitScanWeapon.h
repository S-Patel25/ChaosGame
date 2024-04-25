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

	FVector traceEndWithScatter(const FVector& traceStart, const FVector& hitTarget);
	void weaponTraceHit(const FVector& traceStart, const FVector& hitTarget, FHitResult& outHit);
	
	UPROPERTY(EditAnywhere)
	class UParticleSystem* impactParticles; //to show hit

	UPROPERTY(EditAnywhere)
	USoundCue* hitSound;

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

private:

	UPROPERTY(EditAnywhere)
	UParticleSystem* beamParticles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* muzzleFlash;

	UPROPERTY(EditAnywhere)
	USoundCue* fireSound;
	
	//TRACE END WITH SCATTER (for shotgun stuff)

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float distanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float sphereRadius = 75.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;
};

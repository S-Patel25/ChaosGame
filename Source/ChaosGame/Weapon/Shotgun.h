// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSGAME_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;

protected:

private:

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	uint32 numPellets = 10; //shotgun pellet count


};

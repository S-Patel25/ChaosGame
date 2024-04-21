// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileBullet.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSGAME_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileBullet();


protected:

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override; //dont need ufunction since parent (projectile) class already has it
};

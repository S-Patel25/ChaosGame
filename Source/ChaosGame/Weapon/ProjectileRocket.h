// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSGAME_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket();

	virtual void Destroyed() override; //override so we can have diff properties for the rocket

protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit); //override
	
	virtual void BeginPlay() override;

	void destroyTimerFinished();


	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* trailSystem; //for the rocket trail

	UPROPERTY()
	class UNiagaraComponent* trailSystemComponent;

	UPROPERTY(EditAnywhere)
	USoundCue* projectileLoop; //all for looping sound when rocket in the air

	UPROPERTY()
	UAudioComponent* projectileLoopComponent;

	UPROPERTY(EditAnywhere)
	USoundAttenuation* loopingSoundAttenuation;

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* rocketMesh; //to store the rocket itself

	FTimerHandle destroyTimer;

	UPROPERTY(EditAnywhere)
	float destroyTime = 3.f;
};

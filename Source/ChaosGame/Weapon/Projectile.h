// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class CHAOSGAME_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;
	void startDestroyTimer();
	void destroyTimerFinished();
	void spawnTrailSystem();
	void explodeDamage();

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit); //override

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* impactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* impactSound;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* collisionBox;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* trailSystem; //for the rocket trail

	UPROPERTY()
	class UNiagaraComponent* trailSystemComponent;
	
	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* projectileMovementComponent; //hand ue5 class that handles projectile movememnt (replicated)

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* projectileMesh;

	UPROPERTY(EditAnywhere)
	float DamageInnerRaidus = 200.f;

	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;

private:

	UPROPERTY(EditAnywhere)
	UParticleSystem* tracer;

	UPROPERTY()
	class UParticleSystemComponent* tracerComponent;
	
	FTimerHandle destroyTimer;

	UPROPERTY(EditAnywhere)
	float destroyTime = 3.f;


public:	

	

};

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

private:

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* projectileMovementComponent; //hand ue5 class that handles projectile movememnt (replicated)

	UPROPERTY(EditAnywhere)
	UParticleSystem* tracer;

	UPROPERTY()
	class UParticleSystemComponent* tracerComponent;


public:	

	

};

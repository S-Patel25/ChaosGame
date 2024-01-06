// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"

AProjectile::AProjectile()
{

	PrimaryActorTick.bCanEverTick = true;

	collisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	SetRootComponent(collisionBox);

	collisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic); //since this will be flying through world, set as world dynamic
	collisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	collisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	collisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	collisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic , ECollisionResponse::ECR_Block);

}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ACasing::ACasing()
{
	PrimaryActorTick.bCanEverTick = false;

	casingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TextCasing"));
	SetRootComponent(casingMesh);

	casingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); //so shells dont cause camera collision
	casingMesh->SetSimulatePhysics(true);
	casingMesh->SetEnableGravity(true); //so it falls

	casingMesh->SetNotifyRigidBodyCollision(true); //make sure physics system is the one triggering the hit event

	shellEjectionImpulse = 10.f;

}

void ACasing::BeginPlay()
{
	Super::BeginPlay();
	
	casingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit); //binding function
	casingMesh->AddImpulse(GetActorForwardVector() * shellEjectionImpulse); //will launch in x direction (forward vector)
}

void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (shellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, shellSound, GetActorLocation()); //play shell drop sound
	}

	Destroy();

}

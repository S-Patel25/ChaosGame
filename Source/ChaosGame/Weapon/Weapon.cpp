// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "ChaosGame/Character/ChaosCharacter.h"



AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true; //multiplayer

	weaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	weaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(weaponMesh);

	weaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block); //can bounce when dropped, etc.
	weaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	weaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	areaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	areaSphere->SetupAttachment(RootComponent);
	areaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	areaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); //disabling in constructor, good rule of thumb is to only have settings you want on the SERVER for multiplayer games

	pickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Pickup Widget"));
	pickupWidget->SetupAttachment(weaponMesh);

}
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		areaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		areaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		areaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap); //binding to overlap function
		areaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}

	if (pickupWidget)
	{
		pickupWidget->SetVisibility(false);
	}

}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AChaosCharacter* chaosCharacter = Cast<AChaosCharacter>(OtherActor);

	if (chaosCharacter) //if cast is successful
	{
		//pickupWidget->SetVisibility(true); //will show only on overlap
		chaosCharacter->SetOverlappingWeapon(this); //now that we added the replication code, it will change on all instances (not just server)
	}

}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AChaosCharacter* chaosCharacter = Cast<AChaosCharacter>(OtherActor);

	if (chaosCharacter) //if cast is successful
	{
		chaosCharacter->SetOverlappingWeapon(nullptr); //set to null once done overlap
	}

}

void AWeapon::showPickupWidget(bool bShowWidget)
{
	if (pickupWidget)
	{
		pickupWidget->SetVisibility(bShowWidget);
	}

}


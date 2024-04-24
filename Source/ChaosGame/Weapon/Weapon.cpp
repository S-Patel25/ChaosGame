// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "ChaosGame/Character/ChaosCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Casing.h"
#include "Engine/SkeletalMeshSocket.h"
#include "ChaosGame/PlayerController/ChaosPlayerController.h"


AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true; //multiplayer
	SetReplicateMovement(true); //to avoid inconsisten overlap events with client and server

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

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, weaponState);
	DOREPLIFETIME(AWeapon, Ammo);
}

void AWeapon::setHUDAmmo()
{
	chaosOwnerCharacter = chaosOwnerCharacter == nullptr ? Cast<AChaosCharacter>(GetOwner()) : chaosOwnerCharacter;
	if (chaosOwnerCharacter)
	{
		chaosOwnerController = chaosOwnerController == nullptr ? Cast<AChaosPlayerController>(chaosOwnerCharacter->Controller) : chaosOwnerController;
		if (chaosOwnerController)
		{
			chaosOwnerController->setHUDWeaponAmmo(Ammo);
		}
	}

}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (Owner == nullptr)
	{
		chaosOwnerCharacter = nullptr;
		chaosOwnerController = nullptr;
	}
	else
	{
		setHUDAmmo();
	}
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

void AWeapon::OnRep_Ammo()
{
	setHUDAmmo();
}

void AWeapon::spendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, magCapacity); //making sure ammo can't go below zero or above mag capacity
	setHUDAmmo();
}

void AWeapon::SetWeaponState(EWeaponState state) //weapon properties setter
{
	weaponState = state;

	switch (weaponState)
	{
	case EWeaponState::EWS_Equipped: //use switch to set diff properties on the enum states
		showPickupWidget(false); //set widget to false since now equipped
		getAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision); //no overlap events after equip
		weaponMesh->SetSimulatePhysics(false);
		weaponMesh->SetEnableGravity(false);
		weaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); //setting collision and physics property for weapon

		if (weaponType == EWeaponType::EWT_SMG)
		{
			weaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //for strap physics
			weaponMesh->SetEnableGravity(true);
			weaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}

		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{
			getAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		weaponMesh->SetSimulatePhysics(true);
		weaponMesh->SetEnableGravity(true);
		weaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //setting collision and physics property for weapon
		weaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block); //can bounce when dropped, etc.
		weaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		weaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		break;
	}
}

bool AWeapon::isEmpty()
{
	return Ammo <= 0;
}

void AWeapon::OnRep_WeaponState()
{
	switch (weaponState)
	{
	case EWeaponState::EWS_Equipped: //use switch to set diff properties on the enum states
		showPickupWidget(false);
		areaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); //so overlap events dont happen after weapon has been equipped
		weaponMesh->SetSimulatePhysics(false);
		weaponMesh->SetEnableGravity(false);
		weaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (weaponType == EWeaponType::EWT_SMG)
		{
			weaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //for strap physics
			weaponMesh->SetEnableGravity(true);
			weaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}

		break;
	case EWeaponState::EWS_Dropped:
		weaponMesh->SetSimulatePhysics(true);
		weaponMesh->SetEnableGravity(true);
		weaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //setting collision and physics property for weapon
		weaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block); //can bounce when dropped, etc.
		weaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		weaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		break;
	}

}

void AWeapon::showPickupWidget(bool bShowWidget)
{
	if (pickupWidget)
	{
		pickupWidget->SetVisibility(bShowWidget);
	}

}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (fireAnimation)
	{
		weaponMesh->PlayAnimation(fireAnimation, false); //play anim
	}

	if (casingClass)
	{
		const USkeletalMeshSocket* ammoEjectSocket = weaponMesh->GetSocketByName(FName("AmmoEject"));

		if (ammoEjectSocket)
		{
			FTransform socketTransform = ammoEjectSocket->GetSocketTransform(weaponMesh); 
			UWorld* world = GetWorld();

			{
				world->SpawnActor<ACasing>( //just spawning actor so no need for other params
					casingClass,
					socketTransform.GetLocation(),
					socketTransform.GetRotation().Rotator()
				); //spawn actor
			}

		}
	}
	spendRound();
}

void AWeapon::dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules detachRules(EDetachmentRule::KeepWorld, true);
	weaponMesh->DetachFromComponent(detachRules);
	SetOwner(nullptr); //so weapon has no owner

	chaosOwnerCharacter = nullptr;
	chaosOwnerController = nullptr;

}

void AWeapon::addAmmo(int32 ammoToAdd)
{
	Ammo = FMath::Clamp(Ammo - ammoToAdd, 0, magCapacity);
	setHUDAmmo();
}


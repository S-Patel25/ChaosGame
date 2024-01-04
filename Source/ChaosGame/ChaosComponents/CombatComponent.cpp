// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "ChaosGame/Weapon/Weapon.h"
#include "ChaosGame/Character/ChaosCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming); //this will make sure client gets replication properly for aiming
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, equippedWeapon); //replicate weapon
	DOREPLIFETIME(UCombatComponent, bAiming);
}

void UCombatComponent::equipWeapon(AWeapon* weaponToEquip)
{
	if (chaosCharacter == nullptr || weaponToEquip == nullptr) return; //check first
	
	equippedWeapon = weaponToEquip;

	equippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped); //remember to update enum states

	const USkeletalMeshSocket* handSocket = chaosCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket")); //get the socket

	if (handSocket)
	{
		handSocket->AttachActor(equippedWeapon, chaosCharacter->GetMesh()); //attaches actor to the character
	}

	equippedWeapon->SetOwner(chaosCharacter); //character now is owner of weapon class as we atached and equipped
}


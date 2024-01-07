// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "ChaosGame/Weapon/Weapon.h"
#include "ChaosGame/Character/ChaosCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	baseWalkSpeed = 600.f;
	aimWalkSpeed = 450.f;

}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (chaosCharacter)
	{
		chaosCharacter->GetCharacterMovement()->MaxWalkSpeed = baseWalkSpeed;
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming); //this will make sure client gets replication properly for aiming

	if (chaosCharacter)
	{
		chaosCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? aimWalkSpeed : baseWalkSpeed; //using ternary to easily check if aiming or not to update movement speed
	}

}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (equippedWeapon && chaosCharacter)
	{
		chaosCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		chaosCharacter->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed)
	{
		FHitResult hitResult;
		traceUnderCrosshairs(hitResult);
		ServerFire(hitResult.ImpactPoint); //calling server RPC
	}
}

void UCombatComponent::traceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2d viewportSize;

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(viewportSize); //getting viewport size to make the trace accurate
	}

	FVector2D crosshairLocation(viewportSize.X / 2, viewportSize.Y / 2.f); //this gets center of screen
	
	FVector crosshairWorldPosition;
	FVector crosshairWorldDirection;

	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld( //converts 2d space into 3D world space (so we have accurate location of the center)
		UGameplayStatics::GetPlayerController(this, 0),
		crosshairLocation,
		crosshairWorldPosition,
		crosshairWorldDirection
	);

	if (bScreenToWorld) //once we have center location relative to world, now we perform a line trace
	{
		FVector start = crosshairWorldPosition;
		FVector end = start + crosshairWorldDirection * TRACE_LENGTH; //multiply by big val since its a world direction var is a unit vector (length 1)

		GetWorld()->LineTraceSingleByChannel( //line trace on visibility channel
			TraceHitResult,
			start,
			end,
			ECollisionChannel::ECC_Visibility
		);
	}

}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget); //calling rom the server runs on server AND all clients
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (equippedWeapon == nullptr) return;

	if (chaosCharacter)
	{
		chaosCharacter->playFireMontage(bAiming);
		equippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;

	if (chaosCharacter)
	{
		chaosCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? aimWalkSpeed : baseWalkSpeed; //using ternary to easily check if aiming or not to update movement speed
	}
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

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

	chaosCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	chaosCharacter->bUseControllerRotationYaw = true; //set in server, make sure to use rep notify for client aswell
}


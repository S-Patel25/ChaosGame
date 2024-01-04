// Fill out your copyright notice in the Description page of Project Settings.


#include "ChaosAnimInstance.h"
#include "ChaosCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UChaosAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	chaosCharacter = Cast<AChaosCharacter>(TryGetPawnOwner()); //cast since this returns pawn
}

void UChaosAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime); //make sure to always call super

	if (chaosCharacter == nullptr)
	{
		chaosCharacter = Cast<AChaosCharacter>(TryGetPawnOwner());
	}

	if (chaosCharacter == nullptr)
	{
		return;
	}

	FVector velocity = chaosCharacter->GetVelocity(); //getting speed var from velocity
	velocity.Z = 0.f;
	speed = velocity.Size();


	//use char movement class to get specifics for what you need
	bIsInAir = chaosCharacter->GetCharacterMovement()->IsFalling(); //for jump anims
	bIsAccelerating = chaosCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false; //use ternary to see if accelerating

	bWeaponEquipped = chaosCharacter->isWeaponEquipped(); //so we can change animation pose
	bIsCrouched = chaosCharacter->bIsCrouched;
	bAiming = chaosCharacter->isAiming();
}

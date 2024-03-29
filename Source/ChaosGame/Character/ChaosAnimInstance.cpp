// Fill out your copyright notice in the Description page of Project Settings.


#include "ChaosAnimInstance.h"
#include "ChaosCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ChaosGame/ChaosTypes/CombatState.h"
#include "ChaosGame/Weapon/Weapon.h"

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
	equippedWeapon = chaosCharacter->GetEquippedWeapon();
	bIsCrouched = chaosCharacter->bIsCrouched;
	bAiming = chaosCharacter->isAiming();
	turningInPlace = chaosCharacter->GetTurningInPlace();
	bRotateRootBone = chaosCharacter->shouldRotateRootBone();
	bElimmed = chaosCharacter->isElimmed();

	//yaw offset strafing
	FRotator aimRotation = chaosCharacter->GetBaseAimRotation(); //keep track of aim rotation so we can change yaw and lean
	FRotator movementRotation = UKismetMathLibrary::MakeRotFromX(chaosCharacter->GetVelocity()); //based on where character is moving
	FRotator deltaRot = UKismetMathLibrary::NormalizedDeltaRotator(movementRotation, aimRotation); //used for smooth interp
	deltaRotation = FMath::RInterpTo(deltaRotation, deltaRot, DeltaTime, 6.f); //better then using blendspace interp as it causes jerk behaviour
	yawOffset = deltaRotation.Yaw; //will make it smooth as RInterp takes shortest path
	
	//lean
	characterRotationLastFrame = characterRotation;
	characterRotation = chaosCharacter->GetActorRotation();

	const FRotator delta = UKismetMathLibrary::NormalizedDeltaRotator(characterRotation, characterRotationLastFrame); //get delta
	const float target = delta.Yaw / DeltaTime;
	const float interp = FMath::FInterpTo(lean, target, DeltaTime, 6.f); //to avoid jerkiness when leaning
	lean = FMath::Clamp(interp, -90.f, 90.f); //clamp so it doesn't go beyond and create weird behaviour

	AO_Yaw = chaosCharacter->GetAO_Yaw();
	AO_Pitch = chaosCharacter->GetAO_Pitch();

	if (bWeaponEquipped && equippedWeapon && equippedWeapon->getWeaponMesh() && chaosCharacter->GetMesh())
	{
		leftHandTransform = equippedWeapon->getWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);

		FVector outPosition;
		FRotator outRotation;

		//using transform bone space method to get left hand position in the correct spot
		chaosCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), leftHandTransform.GetLocation(), FRotator::ZeroRotator, outPosition, outRotation);

		leftHandTransform.SetLocation(outPosition);
		leftHandTransform.SetRotation(FQuat(outRotation)); //using set location to align hand bone properly

		if (chaosCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform rightHandTransform = chaosCharacter->GetMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
			FRotator lookAtRotation = UKismetMathLibrary::FindLookAtRotation(rightHandTransform.GetLocation(), rightHandTransform.GetLocation() + (rightHandTransform.GetLocation() - chaosCharacter->getHitTarget())); //vector from transform to hit target (gives backward since hand_r x direction is opposite)
			rightHandRotation = FMath::RInterpTo(rightHandRotation, lookAtRotation, DeltaTime, 30.f);

			rightHandRotation.Roll += chaosCharacter->RightHandRotationRoll; //to fix alignment
			rightHandRotation.Pitch += chaosCharacter->RightHandRotationPitch;
			rightHandRotation.Yaw += chaosCharacter->RightHandRotationYaw;
		}
	}

	bUseFABRIK = chaosCharacter->getCombatState() != ECombatState::ECS_Reloading; //only use FABRIK when not reloading

	bUseAimOffsets = chaosCharacter->getCombatState() != ECombatState::ECS_Reloading && !chaosCharacter->getDisableGameplay(); //to prevent weird aim offset behaviour when reloading

	bTransformRightHand = chaosCharacter->getCombatState() != ECombatState::ECS_Reloading && !chaosCharacter->getDisableGameplay(); //same for now, will change later (with more states involved, grenades, diff weapons, etc.)
};

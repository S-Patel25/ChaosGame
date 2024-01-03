// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ChaosAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSGAME_API UChaosAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	

public:
	virtual void NativeInitializeAnimation() override; //kind of like beginPlay but for anim
	virtual void NativeUpdateAnimation(float DeltaTime) override; //like tick function 

private:
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true")) //need to use meta tag when exposing a priv var to BP's
	class AChaosCharacter* chaosCharacter;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float speed;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir; //check for jump anims

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bWeaponEquipped;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsCrouched;
};

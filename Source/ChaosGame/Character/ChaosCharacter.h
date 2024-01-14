// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h" //we need this because we cannot forward declare a non-pointer variable
#include "Net/UnrealNetwork.h"
#include "ChaosGame/ChaosTypes/TurningInPlace.h"
#include "ChaosGame/Interfaces/InteractWithCrosshairsInterface.h"
#include "ChaosCharacter.generated.h"


class UInputMappingContext; //forward declaring for enhanced input
class UInputAction;


UCLASS()
class CHAOSGAME_API AChaosCharacter : public ACharacter, public IInteractWithCrosshairsInterface //use I version for interfaces
{
	GENERATED_BODY()

public:
	
	AChaosCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; //function override to keep track of replicated variables
	virtual void PostInitializeComponents() override;

	void playFireMontage(bool bAiming);

	//to test weapon rotation

	UPROPERTY(EditAnywhere, Category = "WeaponRotation")
	float RightHandRotationRoll = 180.f;

	UPROPERTY(EditAnywhere, Category = "WeaponRotation")
	float RightHandRotationYaw;

	UPROPERTY(EditAnywhere, Category = "WeaponRotation")
	float RightHandRotationPitch;

protected:
	virtual void BeginPlay() override;

	//INPUT

	UPROPERTY(EditAnywhere, Category = "Input") //make sure to use modifiers
	UInputMappingContext* chaosContext; //using new input system for character movement

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* movementAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* lookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* jumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* equipAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* crouchAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* aimAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* fireAction;


	//enhanced input stuff
	void Movement(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Equip(); //just a key press so no need for the value var
	void CrouchPressed();
	void AimPressed();
	void AimReleased();
	void FirePressed();
	void FireReleased();


	void AimOffset(float DeltaTime);
	virtual void Jump() override;

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* cameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* followCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	class UWidgetComponent* overheadWidget;
	
	UPROPERTY(ReplicatedUsing = OnRep_overlappingWeapon) //new specifier to make sure var is replicated
	class AWeapon* overlappingWeapon;

	UFUNCTION()
	void OnRep_overlappingWeapon(AWeapon* lastWeapon); //rep notifies (only get called from server to client, show changges wont show on server)
	
	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* combat;

	UFUNCTION(Server, Reliable) //reliable means function will and needs be guarnteed on the remote machine (use sparingly)
	void ServerEquipButtonPressed(); //Using RPC (remote procedure calls to handle server and client behaviour, designed on one machine, executed on another

	float AO_Yaw;
	float interpAO_Yaw; //for root bone turning
	float AO_Pitch;
	FRotator startingAimRotation;

	ETurningInPlace turningInPlace;
	void TurnInPlace(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* fireWeaponMontage;


	void hideCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
	float cameraThreshold = 200.f;

public:

	void SetOverlappingWeapon(AWeapon* weapon);
	bool isWeaponEquipped();
	bool isAiming();
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch;  }
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return turningInPlace; }
	FVector getHitTarget() const;
	FORCEINLINE UCameraComponent* getFollowCamera() const { return followCamera; }
};	

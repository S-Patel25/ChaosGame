// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h" //we need this because we cannot forward declare a non-pointer variable
#include "Net/UnrealNetwork.h"
#include "ChaosGame/ChaosTypes/TurningInPlace.h"
#include "ChaosGame/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "ChaosGame/ChaosTypes/CombatState.h"
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

	void playElimMontage();

	void playReloadMontage();

	virtual void OnRep_ReplicatedMovement() override; //to have the sim proxy update faster

	void Elim(); //server elim, one below is for client (hence multicast)

	UFUNCTION(NetMulticast, Reliable)
	void multicastElim();

	virtual void Destroyed() override;

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	//to test weapon rotation

	UPROPERTY(EditAnywhere, Category = "WeaponRotation")
	float RightHandRotationRoll = 180.f;

	UPROPERTY(EditAnywhere, Category = "WeaponRotation")
	float RightHandRotationYaw;

	UPROPERTY(EditAnywhere, Category = "WeaponRotation")
	float RightHandRotationPitch;

	UFUNCTION(BlueprintImplementableEvent)
	void showSniperScopeWidget(bool bShowScope);


protected:
	virtual void BeginPlay() override;

	void updateHUDHealth();

	void playHitReactMontage();

	//HUD intializing
	void pollInit();

	void rotateInPlace(float DeltaTime);

	UFUNCTION() //dont forget UFUNCTION macro for callbacks
	void recieveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser); //specific function callback

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

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* reloadAction;


	//enhanced input stuff
	void Movement(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Equip(); //just a key press so no need for the value var
	void CrouchPressed();
	void AimPressed();
	void AimReleased();
	void FirePressed();
	void FireReleased();
	void ReloadPressed();


	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	void SimProxiesTurn(); //this function is to fix the jitter on sim proxies as anim bp does not update every frame
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	class UCombatComponent* combat;

	UFUNCTION(Server, Reliable) //reliable means function will and needs be guarnteed on the remote machine (use sparingly)
	void ServerEquipButtonPressed(); //Using RPC (remote procedure calls to handle server and client behaviour, designed on one machine, executed on another

	float AO_Yaw;
	float interpAO_Yaw; //for root bone turning
	float AO_Pitch;
	FRotator startingAimRotation;

	ETurningInPlace turningInPlace;
	void TurnInPlace(float DeltaTime);

	//MONTAGE STUFF

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* fireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* hitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* elimMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* reloadMontage;

	void hideCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
	float cameraThreshold = 200.f;

	bool bRotateRootBone;

	float turnThreshold = 0.5f;
	FRotator proxyRotationLastFrame;
	FRotator proxyRotation; //current
	float proxyYaw;
	float timeSinceLastMovementReplication;

	float calculateSpeed(); //refactored method

	//HEALTH STUFF

	UPROPERTY(EditAnywhere, Category = "PlayerStats")
	float maxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "PlayerStats")
	float health = 100.f;

	UFUNCTION()
	void OnRep_Health();

	UPROPERTY()
	class AChaosPlayerController* chaosPlayerController;

	bool bElimmed = false;

	FTimerHandle elimTimer;

	UPROPERTY(EditDefaultsOnly) //should be same for all players
	float elimDelay = 3.f; 

	void elimTimerFinished();

	//DISSOLVE STUFF

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* dissolveTimeline;

	FOnTimelineFloat dissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* dissolveCurve;

	UFUNCTION() //since we're binding to a track
	void updateDissolveMaterial(float dissolveValue);
	void startDissolve();

	UPROPERTY(VisibleAnywhere, Category = "Elim") //dyanmically adjusted at runtime
	UMaterialInstanceDynamic* dynamicDissolveMI;

	UPROPERTY(EditAnywhere, Category = "Elim") //this one is set
	UMaterialInstance* dissolveMaterialInstance;

	//ELIM BOT STUFF

	UPROPERTY(EditAnywhere)
	UParticleSystem* elimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* elimBotComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* elimBotSound;

	UPROPERTY()
	class AChaosPlayerState* chaosPlayerState;


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
	FORCEINLINE bool shouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool isElimmed() const { return bElimmed; }
	FORCEINLINE float getHealth() const { return health; }
	FORCEINLINE float getMaxHealth() const { return maxHealth; }
	ECombatState getCombatState() const;
	FORCEINLINE UCombatComponent* getCombat() const { return combat; }
	FORCEINLINE bool getDisableGameplay() const { return bDisableGameplay; }
};	

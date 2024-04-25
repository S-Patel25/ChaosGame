// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ChaosGame/HUD/ChaosHUD.h"
#include "ChaosGame/Weapon/WeaponTypes.h"
#include "ChaosGame/ChaosTypes/CombatState.h"
#include "CombatComponent.generated.h"

class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CHAOSGAME_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	friend class AChaosCharacter;


	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void equipWeapon(AWeapon* weaponToEquip);

	void Reload();
	
	UFUNCTION(BlueprintCallable)
	void finishReloading();
	
	void FireButtonPressed(bool bPressed);

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable) //reliable since aiming is very important
	void ServerSetAiming(bool bIsAiming); //making a RPC so client has aiming working properly

	UFUNCTION()
	void OnRep_EquippedWeapon();

	

	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget); //RPC so fire effects can be seen
	//net quantize sends info in a efficient matter (whole numbers, not rounded therefore is faster and less bandwidth taken)


	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget); //multicast RPC, allows for ALL clients to get information

	void traceUnderCrosshairs(FHitResult& TraceHitResult); //making a function for the hit target

	void setHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void serverReload();

	void handleReload();

	int32 amountToReload();

private:

	UPROPERTY()
	class AChaosCharacter* chaosCharacter;
	UPROPERTY()
	class AChaosPlayerController* controller;
	UPROPERTY()
	class AChaosHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon) //needs to be replicated so animations can be seen on client aswell as server
	AWeapon* equippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float baseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float aimWalkSpeed;

	bool bFireButtonPressed;


	//HUD STUFF

	float crosshairVelocityFactor;
	float crosshairInAirFactor;
	float crosshairAimFactor;
	float crosshairShootFactor;

	FVector hitTarget;

	FHUDPackage HUDPackage;

	//AIMING STUFF

	float defaultFOV; //not aiming FOV

	UPROPERTY(EditAnywhere, Category = Combat)
	float zoomedFOV = 30.f;

	float currentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float zoomInterpSpeed = 30.f;

	void interpFOV(float DeltaTime);

	//AUTO FIRE STUFF

	FTimerHandle fireTimer;

	bool bCanFire;

	void startFireTimer();
	void fireTimerFinished();

	bool canFire();

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_CarriedAmmo)
	int32 carriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> carriedAmmoMap; //using a map to map ammo to weapon type

	UPROPERTY(EditAnywhere)
	int32 startingARAmmo = 45; //do for each weapon type

	UPROPERTY(EditAnywhere)
	int32 startingRocketAmmo = 4;

	UPROPERTY(EditAnywhere)
	int32 startingPistolAmmo = 15;

	UPROPERTY(EditAnywhere)
	int32 startingSMGAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 startingShotgunAmmo = 8;

	UPROPERTY(EditAnywhere)
	int32 startingSniperAmmo = 5;

	void intializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState combatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void updateAmmoValues();


public:	
	
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"


UENUM(BlueprintType) //so we can use in bp's
enum class EWeaponState : uint8 //for tracking weapon states
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX") //max check
};

UCLASS()
class CHAOSGAME_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void showPickupWidget(bool bShowWidget);

protected:
	virtual void BeginPlay() override;

	UFUNCTION() //since we're binding, must be ufunction
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, 
		bool bFromSweep, 
		const FHitResult& SweepResult); //func signature for overlapping 

	UFUNCTION()
	void OnSphereEndOverlap //make sure to add end overlap aswell along with begin
	(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);


private:
	UPROPERTY(VisibleAnywhere, Category = "Weapons")
    USkeletalMeshComponent* weaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapons")
	class USphereComponent* areaSphere; //for equipping

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere) //using rep notify for enum since it will be used extensivly
	EWeaponState weaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapons")
	class UWidgetComponent* pickupWidget;

public:	
	
	void SetWeaponState(EWeaponState state); //since enum is priv
	FORCEINLINE USphereComponent* getAreaSphere() const { return areaSphere;  }
};

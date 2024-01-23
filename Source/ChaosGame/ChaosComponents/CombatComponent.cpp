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
#include "ChaosGame/PlayerController/ChaosPlayerController.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	baseWalkSpeed = 600.f;
	aimWalkSpeed = 450.f;

}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	PrimaryComponentTick.Target = this;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.SetTickFunctionEnable(true);
	PrimaryComponentTick.RegisterTickFunction(GetComponentLevel());

	controller = Cast<AChaosPlayerController>(chaosCharacter->Controller);

	if (chaosCharacter)
	{
		chaosCharacter->GetCharacterMovement()->MaxWalkSpeed = baseWalkSpeed;

		if (chaosCharacter->getFollowCamera())
		{
			defaultFOV = chaosCharacter->getFollowCamera()->FieldOfView;
			currentFOV = defaultFOV;
		}

		if (chaosCharacter->HasAuthority())
		{
			intializeCarriedAmmo();
		}
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
		equippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped); //remember to update enum states

		const USkeletalMeshSocket* handSocket = chaosCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket")); //get the socket

		if (handSocket)
		{
			handSocket->AttachActor(equippedWeapon, chaosCharacter->GetMesh()); //attaches actor to the character
		}
		chaosCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		chaosCharacter->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed && equippedWeapon)
	{
		Fire();
	}

}

void UCombatComponent::Fire()
{
	if (canFire()) //ammo check
	{
		//bCanFire = false;
		ServerFire(hitTarget); //calling server RPC
		if (equippedWeapon)
		{
			crosshairShootFactor = 0.75f; //better to use = then plus as crosshairs could get bigger and bigger (can also use clamp)
		}
		startFireTimer(); //start here
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

		if (chaosCharacter)
		{
			float distanceToCharacter = (chaosCharacter->GetActorLocation() - start).Size();
			start += crosshairWorldDirection * (distanceToCharacter + 100.f); //making line trace to start a bit in front of character to solve collision issues
		}


		GetWorld()->LineTraceSingleByChannel( //line trace on visibility channel
			TraceHitResult,
			start,
			end,
			ECollisionChannel::ECC_Visibility
		);

		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = end; //so there is always a valid target
		}
	}

	if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>()) //check to see if it implements interface
	{
		HUDPackage.crosshairsColor = FLinearColor::Red;
	}
	else
	{
		HUDPackage.crosshairsColor = FLinearColor::White;
	}

}

void UCombatComponent::setHUDCrosshairs(float DeltaTime)
{
	if (chaosCharacter == nullptr) return; //null check

	controller = controller == nullptr ? Cast<AChaosPlayerController>(chaosCharacter->Controller) : controller; //cast to player controller

	if (controller)
	{
		HUD = HUD == nullptr ? Cast<AChaosHUD>(controller->GetHUD()) : HUD; //same thing here, check for valid then cast

		if (HUD)
		{

			if (equippedWeapon)
			{
				HUDPackage.crosshairsCenter = equippedWeapon->crosshairsCenter;
				HUDPackage.crosshairsLeft = equippedWeapon->crosshairsLeft;
				HUDPackage.crosshairsRight = equippedWeapon->crosshairsRight;
				HUDPackage.crosshairsBottom = equippedWeapon->crosshairsBottom;
				HUDPackage.crosshairsTop = equippedWeapon->crosshairsTop;
			}
			else
			{
				HUDPackage.crosshairsCenter = nullptr; //set to null if not equipped
				HUDPackage.crosshairsLeft = nullptr;
				HUDPackage.crosshairsRight = nullptr;
				HUDPackage.crosshairsBottom = nullptr;
				HUDPackage.crosshairsTop = nullptr;
			}


			//calculate crosshair spread

			// [0,600] -> [0,1] (mapping speed to 0 or 1)

			FVector2D walkSpeedRange(0.f, chaosCharacter->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D velocityMultiplierRange(0.f, 1.f);

			FVector velocity = chaosCharacter->GetVelocity();
			velocity.Z = 0.f;

			crosshairVelocityFactor = FMath::GetMappedRangeValueClamped(walkSpeedRange, velocityMultiplierRange, velocity.Size()); //change range based on velocity of character (.Size is for magnitude)

			if (chaosCharacter->GetCharacterMovement()->IsFalling()) //in air crosshair spread
			{
				crosshairInAirFactor = FMath::FInterpTo(crosshairInAirFactor, 2.25f, DeltaTime, 4.25f); //interpolates smoothly while in air
			}
			else
			{
				crosshairInAirFactor = FMath::FInterpTo(crosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			if (bAiming)
			{
				crosshairAimFactor = FMath::FInterpTo(crosshairAimFactor, -0.45f, DeltaTime, 30.f); //hard coding aim values, can change later if needed (tweak and play with settings)
			}
			else
			{
				crosshairAimFactor = FMath::FInterpTo(crosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			crosshairShootFactor = FMath::FInterpTo(crosshairShootFactor, 0.f, DeltaTime, 40.f);

			HUDPackage.crosshairSpread = 
				0.5f + //add base val so aim crosshairs dont shrink into themselves
				crosshairVelocityFactor + 
				crosshairInAirFactor +
				crosshairAimFactor +
				crosshairShootFactor;

			HUD->SetHUDPackage(HUDPackage); //set crosshairs then call setter for HUD (if equipped)
		}
	}
	
}

void UCombatComponent::handleReload()
{
	chaosCharacter->playReloadMontage();
}

int32 UCombatComponent::amountToReload()
{
	if (equippedWeapon == nullptr) return 0;

	int32 roomInMag = equippedWeapon->getMagCapacity() - equippedWeapon->getAmmo(); //how much is left in the magazine
	
	if (carriedAmmoMap.Contains(equippedWeapon->GetWeaponType()))
	{
		int32 amountCarried = carriedAmmoMap[equippedWeapon->GetWeaponType()];
		int32 least = FMath::Min(roomInMag, amountCarried); //so it doesn't use the higher amount over the roominmag
		return FMath::Clamp(roomInMag, 0, least);
	}

	return 0;
}

void UCombatComponent::serverReload_Implementation()
{
	if (chaosCharacter == nullptr || equippedWeapon == nullptr) return;

	combatState = ECombatState::ECS_Reloading;
	handleReload();

}

void UCombatComponent::updateAmmoValues()
{
	if (chaosCharacter == nullptr || equippedWeapon == nullptr) return;

	int32 reloadAmount = amountToReload();

	if (carriedAmmoMap.Contains(equippedWeapon->GetWeaponType()))
	{
		carriedAmmoMap[equippedWeapon->GetWeaponType()] -= reloadAmount;
		carriedAmmo = carriedAmmoMap[equippedWeapon->GetWeaponType()]; //update ammo
	}

	controller = controller == nullptr ? Cast<AChaosPlayerController>(chaosCharacter->Controller) : controller;

	if (controller)
	{
		controller->setHUDCarriedAmmo(carriedAmmo);
	}

	equippedWeapon->addAmmo(-reloadAmount);
}

void UCombatComponent::OnRep_CombatState()
{
	switch (combatState)
	{
	case ECombatState::ECS_Reloading:
		handleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed)
		{
			Fire();
		}
		break;
	}

}

void UCombatComponent::interpFOV(float DeltaTime)
{
	if (equippedWeapon == nullptr) return;

	if (bAiming)
	{
		currentFOV = FMath::FInterpTo(currentFOV, equippedWeapon->getZoomedFOV(), DeltaTime, equippedWeapon->getZoomIntepSpeed()); //interp to weapon zoom
	}
	else
	{
		currentFOV = FMath::FInterpTo(currentFOV, defaultFOV, DeltaTime, zoomInterpSpeed); //go back to normal once not aiming
	}
	

	if (chaosCharacter && chaosCharacter->getFollowCamera())
	{
		chaosCharacter->getFollowCamera()->SetFieldOfView(currentFOV); //set it based on aim or not
	}
	

}

void UCombatComponent::startFireTimer()
{
	if (equippedWeapon == nullptr || chaosCharacter == nullptr) return;

	chaosCharacter->GetWorldTimerManager().SetTimer( //using timers to get auto fire working
		fireTimer,
		this,
		&UCombatComponent::fireTimerFinished,
		equippedWeapon->fireDelay
	);


}

void UCombatComponent::fireTimerFinished()
{
	if (equippedWeapon == nullptr) return;

	//bCanFire = true;
	if (bFireButtonPressed && equippedWeapon->bAutomatic)
	{
		Fire();
	}

}

bool UCombatComponent::canFire()
{
	if (equippedWeapon == nullptr) return false;

	return !equippedWeapon->isEmpty() && combatState == ECombatState::ECS_Unoccupied; //checking if ammo empty so we can't fire into negative ammo and also not reloading
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	controller = controller == nullptr ? Cast<AChaosPlayerController>(chaosCharacter->Controller) : controller;

	if (controller)
	{
		controller->setHUDCarriedAmmo(carriedAmmo); //display 
	}
}

void UCombatComponent::intializeCarriedAmmo()
{
	carriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, startingARAmmo); //emplace gets rid of temp vals

}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget); //calling rom the server runs on server AND all clients
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (equippedWeapon == nullptr) return;

	if (chaosCharacter && combatState == ECombatState::ECS_Unoccupied)
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

	

	if (chaosCharacter && chaosCharacter->IsLocallyControlled())
	{
		FHitResult hitResult;
		traceUnderCrosshairs(hitResult);
		hitTarget = hitResult.ImpactPoint;

		setHUDCrosshairs(DeltaTime);
		interpFOV(DeltaTime);
	}

}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, equippedWeapon); //replicate weapon
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, carriedAmmo, COND_OwnerOnly); //only owner cares about the max ammo
	DOREPLIFETIME(UCombatComponent, combatState);
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
	equippedWeapon->setHUDAmmo();

	if (carriedAmmoMap.Contains(equippedWeapon->GetWeaponType())) //make sure its the correct weapon type
	{
		carriedAmmo = carriedAmmoMap[equippedWeapon->GetWeaponType()]; //setting carried ammo
	}

	controller = controller == nullptr ? Cast<AChaosPlayerController>(chaosCharacter->Controller) : controller;

	chaosCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	chaosCharacter->bUseControllerRotationYaw = true; //set in server, make sure to use rep notify for client aswell
}

void UCombatComponent::Reload()
{
	if(carriedAmmo > 0 && combatState != ECombatState::ECS_Reloading) //so we cant spam
	{
		serverReload(); //save bandwith by only calling rpc when reload is needed
	}

}

void UCombatComponent::finishReloading()
{
	if (chaosCharacter == nullptr) return;
	
	if (chaosCharacter->HasAuthority())
	{
		combatState = ECombatState::ECS_Unoccupied;
		updateAmmoValues();
	}

	if (bFireButtonPressed)
	{
		Fire();
	}

}


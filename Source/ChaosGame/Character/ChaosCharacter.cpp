// Fill out your copyright notice in the Description page of Project Settings.


#include "ChaosCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "ChaosGame/Weapon/Weapon.h"
#include "ChaosGame/ChaosComponents/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ChaosAnimInstance.h"
#include "ChaosGame/ChaosGame.h"
#include "ChaosGame/PlayerController/ChaosPlayerController.h"

// Sets default values
AChaosCharacter::AChaosCharacter()
{
 	
	PrimaryActorTick.bCanEverTick = true;

	cameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	cameraBoom->SetupAttachment(GetMesh()); //attahcing to mesh insteaf of root so we can work with crouching later on
	cameraBoom->TargetArmLength = 600.f;
	cameraBoom->bUsePawnControlRotation = true; //setting these up in C++, will show in BP

	followCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	followCamera->SetupAttachment(cameraBoom, USpringArmComponent::SocketName);
	followCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true; //movement based on rotation


	overheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	overheadWidget->SetupAttachment(RootComponent);

	combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat Component"));
	combat->SetIsReplicated(true); //make sure to add

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true; //setting this on c++ code aswell as BP's

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh); //mesh is the collision now, compared to pawn
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 850.f, 0.f);


	turningInPlace = ETurningInPlace::ETIP_NotTurning; //make sure no weird behaviour

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f; //how many times it will be replicated (66 and 33 are common used in fast paced shooter games)


}

void AChaosCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); //make sure to always call super

	DOREPLIFETIME_CONDITION(AChaosCharacter, overlappingWeapon, COND_OwnerOnly); //macro for replication with a condition (owner only means for pawn specific, so only player who interacts will be affected)
	DOREPLIFETIME(AChaosCharacter, health);
}

void AChaosCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (combat)
	{
		combat->chaosCharacter = this;
	}

}

void AChaosCharacter::playFireMontage(bool bAiming)
{
	if (combat == nullptr || combat->equippedWeapon == nullptr) return;

	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();

	if (animInstance && fireWeaponMontage)
	{
		animInstance->Montage_Play(fireWeaponMontage); //play montage
		FName sectionName;

		sectionName = bAiming ? FName("RifleAim") : FName("RifleHip"); //play montage based on aiming or not

		animInstance->Montage_JumpToSection(sectionName);
	}


}

void AChaosCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	timeSinceLastMovementReplication = 0.f;
}

void AChaosCharacter::playHitReactMontage()
{
	if (combat == nullptr || combat->equippedWeapon == nullptr) return;

	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();

	if (animInstance && hitReactMontage)
	{
		animInstance->Montage_Play(hitReactMontage); //play montage

		FName sectionName("FromFront");

		animInstance->Montage_JumpToSection(sectionName);
	}
}

void AChaosCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* playerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* subSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(playerController->GetLocalPlayer())) //using the subsystem and making sure its valid so we can use enhanced input
		{
			subSystem->AddMappingContext(chaosContext, 0); //add it to the subsystem
		}
	}

	chaosPlayerController = Cast<AChaosPlayerController>(Controller); //cast to controller class
	
	if (chaosPlayerController)
	{
		chaosPlayerController->setHUDHealth(health, maxHealth); //set health based on characters heatlh
	}
}

void AChaosCharacter::Movement(const FInputActionValue& Value)
{
	const FVector2D moveVector = Value.Get<FVector2D>(); //using the value from the input to decide how and where character movement happens

	const FRotator rotation = Controller->GetControlRotation();
	const FRotator yawRotation(0.f, rotation.Yaw, 0.f); 

	const FVector forwardDirection = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X); //use rot matrixes to determine forward and right vectors for char movement
	AddMovementInput(forwardDirection, moveVector.Y);
	const FVector rightDirection = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(rightDirection, moveVector.X);

}

void AChaosCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D lookVector = Value.Get<FVector2D>();

	AddControllerPitchInput(lookVector.Y);
	AddControllerYawInput(lookVector.X); //using pitch and yaw for mouse movement
}

void AChaosCharacter::Equip()
{
	if (combat)
	{
		if (HasAuthority())
		{
			combat->equipWeapon(overlappingWeapon); //call equip only if valid and has authority (server)
		}
		else
		{
			ServerEquipButtonPressed(); //now rpc is called if player is a client, (implementation keyword only needed for function definition)
		}
	}
}

void AChaosCharacter::CrouchPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch(); //using inhertied methods from char movement component for crouching
	}
	
}

void AChaosCharacter::AimPressed()
{
	if (combat)
	{
		combat->SetAiming(true);
	}

}

void AChaosCharacter::AimReleased()
{
	if (combat)
	{
		combat->SetAiming(false);
	}
}

void AChaosCharacter::FirePressed()
{
	if (combat)
	{
		combat->FireButtonPressed(true);
	}

}

void AChaosCharacter::FireReleased()
{
	if (combat)
	{
		combat->FireButtonPressed(false);
	}
}

void AChaosCharacter::AimOffset(float DeltaTime)
{
	if (combat && combat->equippedWeapon == nullptr) return; //leave function early if no weapon equipped

	float speed = calculateSpeed();

	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (speed == 0.f && !bIsInAir) //standing still and not jumping
	{
		bRotateRootBone = true;
		FRotator currentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator deltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(currentAimRotation, startingAimRotation);
		AO_Yaw = deltaAimRotation.Yaw;
		if (turningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			interpAO_Yaw = AO_Yaw; //default
		}
		bUseControllerRotationYaw = true; //so offset calculations work properly (when standing still)

		TurnInPlace(DeltaTime);
	}

	if (speed > 0.f || bIsInAir) //running or jumping
	{
		bRotateRootBone = false;
		startingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f); //storing the yaw so we can correctly apply offset
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		turningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
	
	CalculateAO_Pitch();
}

void AChaosCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch; //much easier however pitch is changed when sent across network (compressed and positive) so we have to account for that

	if (AO_Pitch > 90.f && !IsLocallyControlled()) //do correction when pitch > 90 degrees
	{
		// map pitch from [270, 360) to [-90, 0) so correct behvaiour is seen in multiplayer

		FVector2D inRange(270.f, 360.f);
		FVector2D outRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(inRange, outRange, AO_Pitch); //handy function to map for us
	}
}

void AChaosCharacter::SimProxiesTurn()
{
	if (combat == nullptr || combat->equippedWeapon == nullptr) return;

	bRotateRootBone = false;

	float speed = calculateSpeed();

	if (speed > 0.f)
	{
		turningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	proxyRotationLastFrame = proxyRotation;
	proxyRotation = GetActorRotation();
	proxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(proxyRotation, proxyRotationLastFrame).Yaw; //get yaw from last frame till current


	if (FMath::Abs(proxyYaw) > turnThreshold)
	{
		if (proxyYaw > turnThreshold)
		{
			turningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (proxyYaw < -turnThreshold)
		{
			turningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			turningInPlace = ETurningInPlace::ETIP_NotTurning;
		}

		return;
	}

	turningInPlace = ETurningInPlace::ETIP_NotTurning;



}

void AChaosCharacter::Jump()
{
	if (bIsCrouched) //doing this so player can jump while crouching
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}


void AChaosCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled()) //since in order (enum are ints, sim proxie is lower then other ones)
	{
		AimOffset(DeltaTime); //every tick
	}
	else
	{
		timeSinceLastMovementReplication += DeltaTime; //so we can call after a while

		if (timeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}

		CalculateAO_Pitch();
	}

	hideCameraIfCharacterClose();
}


void AChaosCharacter::OnRep_overlappingWeapon(AWeapon* lastWeapon)
{
	if (overlappingWeapon)
	{
		overlappingWeapon->showPickupWidget(true);
	}
	if (lastWeapon)
	{
		lastWeapon->showPickupWidget(false); //to keep track of the last weapon
	}
}

void AChaosCharacter::ServerEquipButtonPressed_Implementation() //need to add the implementation for RPC
{
	if (combat)
	{
		combat->equipWeapon(overlappingWeapon); //call equip only if valid and has authority (server)
	}
}

void AChaosCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f) //based on UE logs, char will turn based on yaw position
	{
		turningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		turningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (turningInPlace != ETurningInPlace::ETIP_NotTurning) //now we interp 
	{
		interpAO_Yaw = FMath::FInterpTo(interpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = interpAO_Yaw; //now our ao yaw will be interped to allow for correct root bone movement

		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			turningInPlace = ETurningInPlace::ETIP_NotTurning; //set it back once character position is back to not turning
			startingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void AChaosCharacter::multicastHit_Implementation()
{
	playHitReactMontage();

}

void AChaosCharacter::hideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return;

	if ((followCamera->GetComponentLocation() - GetActorLocation()).Size() < cameraThreshold)
	{
		GetMesh()->SetVisibility(false); //hides char mesh when blocked by wall so player can still see

		if (combat && combat->equippedWeapon && combat->equippedWeapon->getWeaponMesh())
		{
			combat->equippedWeapon->getWeaponMesh()->bOwnerNoSee = true; //bOwnerNoSee is so that owner (player) gets affected only
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);

		if (combat && combat->equippedWeapon && combat->equippedWeapon->getWeaponMesh())
		{
			combat->equippedWeapon->getWeaponMesh()->bOwnerNoSee = false;
		}
	}

}

float AChaosCharacter::calculateSpeed()
{
	FVector velocity = GetVelocity(); //getting speed var from velocity
	velocity.Z = 0.f;
	return velocity.Size();
}

void AChaosCharacter::OnRep_Health()
{


}

void AChaosCharacter::SetOverlappingWeapon(AWeapon* weapon)
{
	if (overlappingWeapon)
	{
		overlappingWeapon->showPickupWidget(false); //call this before setting to make sure server player has correct behaviour
	}

	overlappingWeapon = weapon;

	if (IsLocallyControlled()) //handy function to check if player is the host
	{
		if (overlappingWeapon)
		{
			overlappingWeapon->showPickupWidget(true); //now this will show on server too if player is hosting
		}
	}
}

void AChaosCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//set up action bindings by casting to enhanced input then binding with new component
	if (UEnhancedInputComponent* enhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		enhancedInput->BindAction(movementAction, ETriggerEvent::Triggered, this, &AChaosCharacter::Movement);
		enhancedInput->BindAction(lookAction, ETriggerEvent::Triggered, this, &AChaosCharacter::Look);
		enhancedInput->BindAction(jumpAction, ETriggerEvent::Triggered, this, &AChaosCharacter::Jump);
		enhancedInput->BindAction(equipAction, ETriggerEvent::Triggered, this, &AChaosCharacter::Equip);
		enhancedInput->BindAction(crouchAction, ETriggerEvent::Triggered, this, &AChaosCharacter::CrouchPressed);
		enhancedInput->BindAction(aimAction, ETriggerEvent::Started, this, &AChaosCharacter::AimPressed); //started and complete is same as pressed and released from old input system
		enhancedInput->BindAction(aimAction, ETriggerEvent::Completed, this, &AChaosCharacter::AimReleased);
		enhancedInput->BindAction(fireAction, ETriggerEvent::Started, this, &AChaosCharacter::FirePressed);
		enhancedInput->BindAction(fireAction, ETriggerEvent::Completed, this, &AChaosCharacter::FireReleased);
	}
}

bool AChaosCharacter::isWeaponEquipped()
{
	return (combat && combat->equippedWeapon);
}

bool AChaosCharacter::isAiming()
{
	return (combat && combat->bAiming);
}

AWeapon* AChaosCharacter::GetEquippedWeapon()
{
	if(combat == nullptr) return nullptr;

	return combat->equippedWeapon;
}

FVector AChaosCharacter::getHitTarget() const
{
	if (combat == nullptr) return FVector();

	return combat->hitTarget;
}

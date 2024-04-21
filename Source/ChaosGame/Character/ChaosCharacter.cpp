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
#include "ChaosGame/Gamemode/ChaosGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "ChaosGame/PlayerState/ChaosPlayerState.h"
#include "ChaosGame/Weapon/WeaponTypes.h"

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

	dissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Dissolve Timeline Component"));
}

void AChaosCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); //make sure to always call super

	DOREPLIFETIME_CONDITION(AChaosCharacter, overlappingWeapon, COND_OwnerOnly); //macro for replication with a condition (owner only means for pawn specific, so only player who interacts will be affected)
	DOREPLIFETIME(AChaosCharacter, health);
	DOREPLIFETIME(AChaosCharacter, bDisableGameplay);
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

void AChaosCharacter::playElimMontage()
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();

	if (animInstance && elimMontage)
	{
		animInstance->Montage_Play(elimMontage); //play montage
	}

}

void AChaosCharacter::playReloadMontage()
{
	if (combat == nullptr || combat->equippedWeapon == nullptr) return;

	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();

	if (animInstance && reloadMontage)
	{
		animInstance->Montage_Play(reloadMontage); //play montage

		FName sectionName;

		switch (combat->equippedWeapon->GetWeaponType())
		{
			case EWeaponType::EWT_AssaultRifle: //montage jump based on weapon type
				sectionName = FName("Rifle");
				break;
			case EWeaponType::EWT_RocketLauncher: //will add animation later
				sectionName = FName("Rifle");
				break;
		}

		animInstance->Montage_JumpToSection(sectionName);
	}
}

void AChaosCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	timeSinceLastMovementReplication = 0.f;
}

void AChaosCharacter::Elim()
{
	if (combat && combat->equippedWeapon)
	{
		combat->equippedWeapon->dropped(); //call drop function in elim method
	}

	multicastElim();

	GetWorldTimerManager().SetTimer(
		elimTimer,
		this,
		&AChaosCharacter::elimTimerFinished,
		elimDelay
	);

}

void AChaosCharacter::Destroyed()
{
	Super::Destroyed();

	if (elimBotComponent) 
	{
		elimBotComponent->DestroyComponent(); //so it doesnt stick around on both server and client (since destroyed works on all machines)
	}

	AChaosGameMode* chaosGameMode = Cast<AChaosGameMode>(UGameplayStatics::GetGameMode(this));

	bool bMatchNotInProgress = chaosGameMode && chaosGameMode->GetMatchState() != MatchState::InProgress;

	if (combat && combat->equippedWeapon && bMatchNotInProgress)
	{
		combat->equippedWeapon->Destroy(); //wont hang around during cooldown state
	}
}


void AChaosCharacter::multicastElim_Implementation()
{
	bElimmed = true;
	playElimMontage(); //will play when character health is 0


	//starting the dissolve effect 
	if (dissolveMaterialInstance)
	{
		dynamicDissolveMI = UMaterialInstanceDynamic::Create(dissolveMaterialInstance, this);

		GetMesh()->SetMaterial(0, dynamicDissolveMI); //set dynamic mesh material

		dynamicDissolveMI->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		dynamicDissolveMI->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}

	startDissolve();

	//after dissolve we disable movement
	
	GetCharacterMovement()->DisableMovement(); //movement stopped
	GetCharacterMovement()->StopMovementImmediately(); //rotation stopped

	bDisableGameplay = true; //use bool instead of disable input
	if (combat)
	{
		combat->FireButtonPressed(false);
	}

	//disable collision

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	//spawn elim bot

	if (elimBotEffect)
	{
		FVector elimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f); //spawn 200 units above player

		elimBotComponent = UGameplayStatics::SpawnEmitterAtLocation( //spawn the particle system
			GetWorld(),
			elimBotEffect,
			elimBotSpawnPoint,
			GetActorRotation()
		);
	}

	if (elimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			elimBotSound,
			GetActorLocation()
		);
	}
}

void AChaosCharacter::elimTimerFinished()
{
	AChaosGameMode* chaosGameMode = GetWorld()->GetAuthGameMode<AChaosGameMode>();

	if (chaosGameMode)
	{
		chaosGameMode->requestRespawn(this, Controller);
	}
}

void AChaosCharacter::playHitReactMontage()
{
	if (combat == nullptr || combat->equippedWeapon == nullptr) return;

	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();

	if (animInstance && hitReactMontage && !animInstance->IsAnyMontagePlaying()) //check to prevent multiple montages playing
	{
		animInstance->Montage_Play(hitReactMontage); //play montage
		FName sectionName("FromFront");
		animInstance->Montage_JumpToSection(sectionName);
	}
}

void AChaosCharacter::pollInit()
{
	if (chaosPlayerState == nullptr)
	{
		chaosPlayerState = GetPlayerState<AChaosPlayerState>(); //function returns player state (no need to cast)

		if (chaosPlayerState)
		{
			chaosPlayerState->addToScore(0.f);
			chaosPlayerState->addToDefeats(0);
		}
	}

}

void AChaosCharacter::rotateInPlace(float DeltaTime)
{
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false; //make sure states and bools are set correctly too before returning
		turningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

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
}

void AChaosCharacter::recieveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	health = FMath::Clamp(health - Damage, 0.f, maxHealth);
	updateHUDHealth();
	playHitReactMontage(); //played on server (make sure to put in OnRep method aswell

	if (health == 0.f) //only do when player eliminated
	{
		AChaosGameMode* chaosGameMode = GetWorld()->GetAuthGameMode<AChaosGameMode>(); //auth game mode gets game mode

		if (chaosGameMode)
		{
			chaosPlayerController = chaosPlayerController == nullptr ? Cast<AChaosPlayerController>(Controller) : chaosPlayerController;
			AChaosPlayerController* attackerController = Cast<AChaosPlayerController>(InstigatorController); //who does the damage

			chaosGameMode->playerEliminated(this, chaosPlayerController, attackerController);
		}
	}
	
}

void AChaosCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (const ULocalPlayer* Player = (GEngine && GetWorld()) ? GEngine->GetFirstGamePlayer(GetWorld()) : nullptr) //quick fix as server player could not move after spawning from warm up stage
	{
		if (UEnhancedInputLocalPlayerSubsystem* subSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Player)) //using the subsystem and making sure its valid so we can use enhanced input
		{
			subSystem->AddMappingContext(chaosContext, 0); //add it to the subsystem
		}
	}

	updateHUDHealth();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AChaosCharacter::recieveDamage); //bind to damage delegate
	}
}

void AChaosCharacter::updateHUDHealth()
{
	chaosPlayerController = chaosPlayerController == nullptr ? Cast<AChaosPlayerController>(Controller) : chaosPlayerController; //cast to controller class

	if (chaosPlayerController)
	{
		chaosPlayerController->setHUDHealth(health, maxHealth); //set health based on characters heatlh
	}
}

void AChaosCharacter::Movement(const FInputActionValue& Value)
{
	if (bDisableGameplay) return; //disable movement if bool true

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
	if (bDisableGameplay) return;

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
	if (bDisableGameplay) return;

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
	if (bDisableGameplay) return;

	if (combat)
	{
		combat->SetAiming(true);
	}

}

void AChaosCharacter::AimReleased()
{
	if (bDisableGameplay) return;

	if (combat)
	{
		combat->SetAiming(false);
	}
}

void AChaosCharacter::FirePressed()
{
	if (bDisableGameplay) return;

	if (combat)
	{
		combat->FireButtonPressed(true);
	}

}

void AChaosCharacter::FireReleased()
{
	if (bDisableGameplay) return;

	if (combat)
	{
		combat->FireButtonPressed(false);
	}
}

void AChaosCharacter::ReloadPressed()
{
	if (bDisableGameplay) return;

	if (combat)
	{
		combat->Reload();
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
	if (bDisableGameplay) return;

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

	rotateInPlace(DeltaTime);
	hideCameraIfCharacterClose();
	pollInit(); //can't do in begin play as player state is not intialized by then, so tick will take care of that
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
	updateHUDHealth();

	if (!bElimmed)
	{
		playHitReactMontage(); //health is replicated, so better to use rep notify then RPC everytime (takes up bandwidth)
	}
	
}


void AChaosCharacter::updateDissolveMaterial(float dissolveValue)
{
	if (dynamicDissolveMI)
	{
		dynamicDissolveMI->SetScalarParameterValue(TEXT("Dissolve"), dissolveValue);
	}

}

void AChaosCharacter::startDissolve()
{
	dissolveTrack.BindDynamic(this, &AChaosCharacter::updateDissolveMaterial); //binding dynamic delegate

	if (dissolveCurve && dissolveTimeline)
	{
		dissolveTimeline->AddInterpFloat(dissolveCurve, dissolveTrack); //makes sure timeline uses the curve
		dissolveTimeline->Play();
	}


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
		enhancedInput->BindAction(reloadAction, ETriggerEvent::Triggered, this, &AChaosCharacter::ReloadPressed);
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

ECombatState AChaosCharacter::getCombatState() const
{
	if (combat == nullptr) return ECombatState::ECS_MAX;

	return combat->combatState;
}

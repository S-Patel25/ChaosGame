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
}

void AChaosCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); //make sure to always call super

	DOREPLIFETIME_CONDITION(AChaosCharacter, overlappingWeapon, COND_OwnerOnly); //macro for replication with a condition (owner only means for pawn specific, so only player who interacts will be affected)

}

void AChaosCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (combat)
	{
		combat->character = this; //setting
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

void AChaosCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
		enhancedInput->BindAction(jumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		enhancedInput->BindAction(equipAction, ETriggerEvent::Triggered, this, &AChaosCharacter::Equip);
	}
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h" //we need this because we cannot forward declare a non-pointer variable
#include "Net/UnrealNetwork.h"
#include "ChaosCharacter.generated.h"


class UInputMappingContext; //forward declaring for enhanced input
class UInputAction;


UCLASS()
class CHAOSGAME_API AChaosCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	
	AChaosCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; //function override to keep track of replicated variables

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

	void Movement(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

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
	
public:

	void SetOverlappingWeapon(AWeapon* weapon);
};

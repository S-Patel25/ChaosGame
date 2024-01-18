// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ChaosPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSGAME_API AChaosPlayerController : public APlayerController
{
	GENERATED_BODY()
	

public:

	void setHUDHealth(float health, float maxHealth);
	virtual void OnPossess(APawn* InPawn) override;

protected:
	virtual void BeginPlay() override;

private:
	class AChaosHUD* chaosHUD;
};

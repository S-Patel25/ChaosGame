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
	void setHUDScore(float score);
	void setHUDDefeats(int32 defeats);
	void setHUDWeaponAmmo(int32 Ammo);
	void setHUDCarriedAmmo(int32 Ammo);
	void setHUDMatchCountdown(float countdownTime);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	void setHUDTime();

private:

	UPROPERTY()
	class AChaosHUD* chaosHUD;

	float matchTime = 120.f;
	uint32 countdownInt = 0.f;
};

// Fill out your copyright notice in the Description page of Project Settings.


#include "ChaosGameMode.h"
#include "ChaosGame/Character/ChaosCharacter.h"
#include "ChaosGame/PlayerController/ChaosPlayerController.h"

void AChaosGameMode::playerEliminated(AChaosCharacter* elimmedCharacter, AChaosPlayerController* victimController, AChaosPlayerController* attackerController)
{
	if (elimmedCharacter)
	{
		elimmedCharacter->elim();
	}

}

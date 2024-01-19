// Fill out your copyright notice in the Description page of Project Settings.


#include "ChaosPlayerState.h"
#include "ChaosGame/Character/ChaosCharacter.h"
#include "ChaosGame/PlayerController/ChaosPlayerController.h"

void AChaosPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	chaosCharacter = chaosCharacter == nullptr ? Cast<AChaosCharacter>(GetPawn()) : chaosCharacter; //casting to pawn

	if (chaosCharacter)
	{
		chaosController = chaosController == nullptr ? Cast<AChaosPlayerController>(chaosCharacter->Controller) : chaosController;

		if (chaosController)
		{
			chaosController->setHUDScore(Score);
		}
	}

}

void AChaosPlayerState::addToScore(float scoreAmount)
{
	Score += scoreAmount;

	chaosCharacter = chaosCharacter == nullptr ? Cast<AChaosCharacter>(GetPawn()) : chaosCharacter; //casting to pawn

	if (chaosCharacter)
	{
		chaosController = chaosController == nullptr ? Cast<AChaosPlayerController>(chaosCharacter->Controller) : chaosController;

		if (chaosController)
		{
			chaosController->setHUDScore(Score);
		}
	}

}

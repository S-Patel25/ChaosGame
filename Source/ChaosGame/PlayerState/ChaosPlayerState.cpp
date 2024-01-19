// Fill out your copyright notice in the Description page of Project Settings.


#include "ChaosPlayerState.h"
#include "ChaosGame/Character/ChaosCharacter.h"
#include "ChaosGame/PlayerController/ChaosPlayerController.h"
#include "Net/UnrealNetwork.h"

void AChaosPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AChaosPlayerState, defeats); //now defeats is replicated

}

void AChaosPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	chaosCharacter = chaosCharacter == nullptr ? Cast<AChaosCharacter>(GetPawn()) : chaosCharacter; //casting to pawn

	if (chaosCharacter)
	{
		chaosController = chaosController == nullptr ? Cast<AChaosPlayerController>(chaosCharacter->Controller) : chaosController;

		if (chaosController)
		{
			chaosController->setHUDScore(GetScore());
		}
	}

}

void AChaosPlayerState::OnRep_Defeats()
{
	chaosCharacter = chaosCharacter == nullptr ? Cast<AChaosCharacter>(GetPawn()) : chaosCharacter; //casting to pawn

	if (chaosCharacter)
	{
		chaosController = chaosController == nullptr ? Cast<AChaosPlayerController>(chaosCharacter->Controller) : chaosController;

		if (chaosController)
		{
			chaosController->setHUDDefeats(defeats);
		}
	}
}

void AChaosPlayerState::addToScore(float scoreAmount)
{
	SetScore(GetScore() + scoreAmount);

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

void AChaosPlayerState::addToDefeats(int32 defeatsAmount)
{
	defeats += defeatsAmount;

	chaosCharacter = chaosCharacter == nullptr ? Cast<AChaosCharacter>(GetPawn()) : chaosCharacter; //casting to pawn

	if (chaosCharacter)
	{
		chaosController = chaosController == nullptr ? Cast<AChaosPlayerController>(chaosCharacter->Controller) : chaosController;

		if (chaosController)
		{
			chaosController->setHUDDefeats(defeats);
		}
	}
}

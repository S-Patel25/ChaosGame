// Fill out your copyright notice in the Description page of Project Settings.


#include "ChaosGameState.h"
#include "Net/UnrealNetwork.h"
#include "ChaosGame/PlayerState/ChaosPlayerState.h"

void AChaosGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AChaosGameState, topScoringPlayers);
}

void AChaosGameState::updateTopScore(AChaosPlayerState* scoringPlayer)
{
	if (topScoringPlayers.Num() == 0)
	{
		topScoringPlayers.Add(scoringPlayer); //if no one, add
		topScore = scoringPlayer->GetScore();
	}
	else if (scoringPlayer->GetScore() == topScore) //tie
	{
		topScoringPlayers.AddUnique(scoringPlayer); //add unique does not add duplicates
	}
	else if (scoringPlayer->GetScore() > topScore)
	{
		topScoringPlayers.Empty(); //empty array if new score leader
		topScoringPlayers.AddUnique(scoringPlayer);
		topScore = scoringPlayer->GetScore();
	}
	
}

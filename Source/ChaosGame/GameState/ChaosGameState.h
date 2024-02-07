// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ChaosGameState.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSGAME_API AChaosGameState : public AGameState
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void updateTopScore(class AChaosPlayerState* scoringPlayer);


	UPROPERTY(Replicated)
	TArray<AChaosPlayerState*> topScoringPlayers; //to keep track of score stats

protected:

private:

	float topScore = 0.f;

public:

	
};

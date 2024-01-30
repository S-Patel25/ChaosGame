// Fill out your copyright notice in the Description page of Project Settings.


#include "ChaosGameMode.h"
#include "ChaosGame/Character/ChaosCharacter.h"
#include "ChaosGame/PlayerController/ChaosPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "ChaosGame/PlayerState/ChaosPlayerState.h"

AChaosGameMode::AChaosGameMode()
{
	bDelayedStart = true; //will set to true, so manually can start match (this allows for warmup stage before match starts) 

}

void AChaosGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart) //use matchstate namespace from gamemode class
	{
		countdownTime = warmupTime - GetWorld()->GetTimeSeconds() + levelStartingTime;

		if (countdownTime <= 0.f)
		{
			StartMatch();
		}
	}

}


void AChaosGameMode::BeginPlay()
{
	Super::BeginPlay();

	levelStartingTime = GetWorld()->GetTimeSeconds(); //for current level time, rather then full game time since launched

}

void AChaosGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator it = GetWorld()->GetPlayerControllerIterator(); it; ++it) //iterator to go through all players
	{
		AChaosPlayerController* chaosPlayer = Cast<AChaosPlayerController>(*it); //deferencing

		if (chaosPlayer)
		{
			chaosPlayer->onMatchStateSet(MatchState); //will set state for all players
		}
	}

}


void AChaosGameMode::playerEliminated(AChaosCharacter* elimmedCharacter, AChaosPlayerController* victimController, AChaosPlayerController* attackerController)
{
	AChaosPlayerState* attackerPlayerState = attackerController ? Cast<AChaosPlayerState>(attackerController->PlayerState) : nullptr; //casts to get attacker and victims player states
	AChaosPlayerState* victimPlayerState = victimController ? Cast<AChaosPlayerState>(victimController->PlayerState) : nullptr;

	if (attackerPlayerState && attackerPlayerState != victimPlayerState)
	{
		attackerPlayerState->addToScore(1.f); //when player gets elimmed
	}

	if (victimPlayerState)
	{
		victimPlayerState->addToDefeats(1); //same as attacker, but will count when player dies
	}


	if (elimmedCharacter)
	{
		elimmedCharacter->Elim();
	}

}

void AChaosGameMode::requestRespawn(ACharacter* elimmedCharacter, AController* elimmedController)
{
	if (elimmedCharacter)
	{
		elimmedCharacter->Reset(); //does alot like detaching from controller, etc.
		elimmedCharacter->Destroy();
	}

	if (elimmedController) //add player start algo later (so you dont end up spawning at the same place)
	{
		TArray<AActor*> playerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), playerStarts); //getting player starts to spawn the player randomly after elimmed
		int32 selection = FMath::RandRange(0, playerStarts.Num() - 1);

		RestartPlayerAtPlayerStart(elimmedController, playerStarts[selection]); //essentially a player respawn method

	}
	
}
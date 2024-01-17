// Fill out your copyright notice in the Description page of Project Settings.


#include "ChaosGameMode.h"
#include "ChaosGame/Character/ChaosCharacter.h"
#include "ChaosGame/PlayerController/ChaosPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

void AChaosGameMode::playerEliminated(AChaosCharacter* elimmedCharacter, AChaosPlayerController* victimController, AChaosPlayerController* attackerController)
{
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

// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 numPlayers = GameState.Get()->PlayerArray.Num(); //use gamestate get method to check num players joined
	
	if(numPlayers == 2) //for testing purposes
	{
		UWorld* world = GetWorld();
		if (world)
		{
			bUseSeamlessTravel = true; //this is to enable seamless travel
			world->ServerTravel(FString("/Game/Maps/NewChaosMap?listen")); //travel to new map once correct amount of players joined ('?' is so we can specify its a listen server)
		}
	}
}

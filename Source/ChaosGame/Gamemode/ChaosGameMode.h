// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ChaosGameMode.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSGAME_API AChaosGameMode : public AGameMode
{
	GENERATED_BODY()
public:

	virtual void playerEliminated(class AChaosCharacter* elimmedCharacter, class AChaosPlayerController* victimController, class AChaosPlayerController* attackerController);

	virtual void requestRespawn(ACharacter* elimmedCharacter, AController* elimmedController);

protected:


private:


};

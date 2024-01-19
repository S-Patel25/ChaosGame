// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ChaosPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSGAME_API AChaosPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void OnRep_Score() override;

	void addToScore(float scoreAmount);


protected:

private:

	class AChaosCharacter* chaosCharacter;
	class AChaosPlayerController* chaosController;

};

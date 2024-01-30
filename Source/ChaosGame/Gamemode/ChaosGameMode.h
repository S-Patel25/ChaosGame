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

	AChaosGameMode();

	virtual void Tick(float DeltaTime) override;

	virtual void playerEliminated(class AChaosCharacter* elimmedCharacter, class AChaosPlayerController* victimController, class AChaosPlayerController* attackerController);
	virtual void requestRespawn(ACharacter* elimmedCharacter, AController* elimmedController);

	UPROPERTY(EditDefaultsOnly)
	float warmupTime = 10.f; //how much time we can fly around before match starts (warm up)

	float levelStartingTime = 0.f;

protected:

	virtual void BeginPlay() override;

private:

	float countdownTime = 0.f;

};

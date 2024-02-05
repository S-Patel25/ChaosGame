// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ChaosGameMode.generated.h"

namespace MatchState
{
	extern CHAOSGAME_API const FName Cooldown; //adding custom match state for end of match, display player stats, etc.
}

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

	UPROPERTY(EditDefaultsOnly)
	float matchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float cooldownTime = 120.f;


	float levelStartingTime = 0.f;

protected:

	virtual void BeginPlay() override;

	virtual void OnMatchStateSet() override;

private:

	float countdownTime = 0.f;

};

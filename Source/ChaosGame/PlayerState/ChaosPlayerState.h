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

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	//REP NOTIFY STUFF

	virtual void OnRep_Score() override;

	UFUNCTION() //dont forget
	virtual void OnRep_Defeats();

	void addToScore(float scoreAmount);
	void addToDefeats(int32 defeatsAmount);


protected:

private:

	UPROPERTY() //intializes pointer (can also do == nullptr), good practice to do this as not can cause weird issues sometimes
	class AChaosCharacter* chaosCharacter;

	UPROPERTY()
	class AChaosPlayerController* chaosController;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 defeats;

};

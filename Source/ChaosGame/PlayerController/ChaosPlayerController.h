// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ChaosPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSGAME_API AChaosPlayerController : public APlayerController
{
	GENERATED_BODY()
	

public:

	void setHUDHealth(float health, float maxHealth);
	void setHUDScore(float score);
	void setHUDDefeats(int32 defeats);
	void setHUDWeaponAmmo(int32 Ammo);
	void setHUDCarriedAmmo(int32 Ammo);
	void setHUDMatchCountdown(float countdownTime);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual float getServerTime(); //synced with world time

	virtual void ReceivedPlayer() override; //so we can sync the client and server as soon as possible

	void onMatchStateSet(FName state);

	void handleMatchHasStarted(); //similar to gamemode class


protected:
	virtual void BeginPlay() override;

	void setHUDTime();

	void pollInit(); //initialize stuff

	//ROUND TRIP TIME CODE STUFF(sync time between client and server)

	UFUNCTION(Server, Reliable) //requests current server time
	void ServerRequestServerTime(float timeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float timeOfClientRequest, float timeServerRecievedClientRequest); //sends back server time to client

	float clientServerDelta = 0.f; //difference between client and server time

	UPROPERTY(EditAnywhere, Category = "Time")
	float timeSyncFrequency = 5.f;

	float timeSyncRunningTime = 0.f;
	void checkTimeSync(float DeltaTime);

private:

	UPROPERTY()
	class AChaosHUD* chaosHUD;

	float matchTime = 120.f;
	uint32 countdownInt = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_matchState)
	FName matchState;

	UFUNCTION()
	void OnRep_matchState();

	UPROPERTY()
	class UCharacterOverlay* characterOverlay;

	bool bIntializeCharacterOverlay = false;

	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDDefeats;



};

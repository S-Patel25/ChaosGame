// Fill out your copyright notice in the Description page of Project Settings.


#include "ChaosPlayerController.h"
#include "ChaosGame/HUD/ChaosHUD.h"
#include "ChaosGame/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "ChaosGame/Character/ChaosCharacter.h"

void AChaosPlayerController::BeginPlay()
{
	Super::BeginPlay();

	chaosHUD = Cast<AChaosHUD>(GetHUD()); //casting as gethud returns AHUD
}

void AChaosPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	setHUDTime();

	checkTimeSync(DeltaTime);
}


void AChaosPlayerController::checkTimeSync(float DeltaTime)
{
	timeSyncRunningTime += DeltaTime;

	if (IsLocalController() && timeSyncRunningTime > timeSyncFrequency) //to prevent drifting of times
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		timeSyncRunningTime = 0.f; //reset
	}
}


void AChaosPlayerController::setHUDTime() //should be done in player gamemode, just doing this for testing for now
{
	uint32 secondsLeft = FMath::CeilToInt(matchTime - getServerTime()); 

	if (countdownInt != secondsLeft)
	{
		setHUDMatchCountdown(matchTime - getServerTime());
	}

	countdownInt = secondsLeft;

}

void AChaosPlayerController::ServerRequestServerTime_Implementation(float timeOfClientRequest)
{
	float serverTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(timeOfClientRequest, serverTimeOfReceipt); //send client the server time
}

void AChaosPlayerController::ClientReportServerTime_Implementation(float timeOfClientRequest, float timeServerRecievedClientRequest)
{
	float roundTripTime = GetWorld()->GetTimeSeconds() - timeOfClientRequest;
	float currentServerTime = timeServerRecievedClientRequest + (0.5f * roundTripTime);

	clientServerDelta = currentServerTime - GetWorld()->GetTimeSeconds();
}

float AChaosPlayerController::getServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();


	return GetWorld()->GetTimeSeconds() + clientServerDelta;
}

void AChaosPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}

}

void AChaosPlayerController::setHUDHealth(float health, float maxHealth)
{
	chaosHUD = chaosHUD == nullptr ? Cast<AChaosHUD>(GetHUD()) : chaosHUD; //efficient way to check

	bool bHUDValid = chaosHUD && 
		chaosHUD->characterOverlay && 
		chaosHUD->characterOverlay->HealthBar && 
		chaosHUD->characterOverlay->HealthText;

	if (bHUDValid)
	{
		const float healthPercent = health / maxHealth;
		chaosHUD->characterOverlay->HealthBar->SetPercent(healthPercent); //set val of progress bar based on health
		FString healthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(health), FMath::CeilToInt(maxHealth)); //will show health and maxhealth (%d)

		chaosHUD->characterOverlay->HealthText->SetText(FText::FromString(healthText));
	}
}

void AChaosPlayerController::setHUDScore(float score)
{
	chaosHUD = chaosHUD == nullptr ? Cast<AChaosHUD>(GetHUD()) : chaosHUD;

	bool bHUDValid = chaosHUD &&
		chaosHUD->characterOverlay &&
		chaosHUD->characterOverlay->ScoreAmount;

	if (bHUDValid)
	{
		FString scoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(score));
		chaosHUD->characterOverlay->ScoreAmount->SetText(FText::FromString(scoreText));
	}
}

void AChaosPlayerController::setHUDDefeats(int32 defeats)
{

	chaosHUD = chaosHUD == nullptr ? Cast<AChaosHUD>(GetHUD()) : chaosHUD;

	bool bHUDValid = chaosHUD &&
		chaosHUD->characterOverlay &&
		chaosHUD->characterOverlay->DefeatAmount;

	if (bHUDValid)
	{
		FString defeatText = FString::Printf(TEXT("%d"), defeats);
		chaosHUD->characterOverlay->DefeatAmount->SetText(FText::FromString(defeatText));
	}
}

void AChaosPlayerController::setHUDWeaponAmmo(int32 Ammo)
{
	chaosHUD = chaosHUD == nullptr ? Cast<AChaosHUD>(GetHUD()) : chaosHUD;

	bool bHUDValid = chaosHUD &&
		chaosHUD->characterOverlay &&
		chaosHUD->characterOverlay->WeapAmmoAmount;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		chaosHUD->characterOverlay->WeapAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void AChaosPlayerController::setHUDCarriedAmmo(int32 Ammo)
{
	chaosHUD = chaosHUD == nullptr ? Cast<AChaosHUD>(GetHUD()) : chaosHUD;

	bool bHUDValid = chaosHUD &&
		chaosHUD->characterOverlay &&
		chaosHUD->characterOverlay->CarriedAmmoAmount;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		chaosHUD->characterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}

}

void AChaosPlayerController::setHUDMatchCountdown(float countdownTime)
{
	chaosHUD = chaosHUD == nullptr ? Cast<AChaosHUD>(GetHUD()) : chaosHUD;

	bool bHUDValid = chaosHUD &&
		chaosHUD->characterOverlay &&
		chaosHUD->characterOverlay->MatchCountdownText;

	if (bHUDValid)
	{
		int32 minutes = FMath::FloorToInt(countdownTime / 60.f); //this will get minutes
		int32 seconds = countdownTime - minutes * 60; //gets seconds

		FString countdownText = FString::Printf(TEXT("%02d:%02d"), minutes, seconds); //will show time with correct formatting (10:03), for example
		chaosHUD->characterOverlay->MatchCountdownText->SetText(FText::FromString(countdownText));
	}
}

void AChaosPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AChaosCharacter* chaosCharacter = Cast<AChaosCharacter>(InPawn); //cast to pawn

	if (chaosCharacter)
	{
		setHUDHealth(chaosCharacter->getHealth(), chaosCharacter->getMaxHealth());
	}


}
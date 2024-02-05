// Fill out your copyright notice in the Description page of Project Settings.


#include "ChaosPlayerController.h"
#include "ChaosGame/HUD/ChaosHUD.h"
#include "ChaosGame/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "ChaosGame/Character/ChaosCharacter.h"
#include "Net/UnrealNetwork.h"
#include "ChaosGame/Gamemode/ChaosGameMode.h"
#include "ChaosGame/HUD/Announcement.h"
#include "Kismet/GameplayStatics.h"

void AChaosPlayerController::BeginPlay()
{
	Super::BeginPlay();

	chaosHUD = Cast<AChaosHUD>(GetHUD()); //casting as gethud returns AHUD
	ServerCheckMatchState();
}

void AChaosPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	setHUDTime();

	checkTimeSync(DeltaTime);

	pollInit();
}

void AChaosPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AChaosPlayerController, matchState); //now replicated
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


void AChaosPlayerController::ServerCheckMatchState_Implementation()
{
	AChaosGameMode* gameMode = Cast<AChaosGameMode>(UGameplayStatics::GetGameMode(this)); //cast to gamemode

	if (gameMode)
	{
		warmupTime = gameMode->warmupTime;
		matchTime = gameMode->matchTime; //setting through gamemode now rather then hardcoding
		levelStartingTime = gameMode->levelStartingTime;
		matchState = gameMode->GetMatchState();

		ClientJoinMidgame(matchState, warmupTime, matchTime, levelStartingTime);

		if (chaosHUD && matchState == MatchState::WaitingToStart)
		{
			chaosHUD->addAnnouncement(); //do it in begin play instead of waitingtostart state as HUD wont be intialized then
		}
	}

}

void AChaosPlayerController::ClientJoinMidgame_Implementation(FName stateOfMatch, float warmup, float match, float startingTime)
{
	warmupTime = warmup;
	matchTime = match;
	levelStartingTime = startingTime;
	matchState = stateOfMatch;

	onMatchStateSet(matchState);

	if (chaosHUD && matchState == MatchState::WaitingToStart)
	{
		chaosHUD->addAnnouncement(); //do it in begin play instead of waitingtostart state as HUD wont be intialized then
	}

}

void AChaosPlayerController::setHUDTime() //should be done in player gamemode, just doing this for testing for now
{
	float timeLeft = 0.f;
	if (matchState == MatchState::WaitingToStart) timeLeft = warmupTime - getServerTime() + levelStartingTime; //simple math to get correct time to display on player HUD
	else if (matchState == MatchState::InProgress) timeLeft = warmupTime + matchTime - getServerTime() + levelStartingTime;

	uint32 secondsLeft = FMath::CeilToInt(matchTime - getServerTime()); 

	if (countdownInt != secondsLeft)
	{
		if (matchState == MatchState::WaitingToStart)
		{
			setHUDAnnouncementCountdown(timeLeft);
		}
		if (matchState == MatchState::InProgress)
		{
			setHUDMatchCountdown(timeLeft);
		}
	}

	countdownInt = secondsLeft;

}

void AChaosPlayerController::pollInit()
{
	if (characterOverlay == nullptr)
	{
		if (chaosHUD && chaosHUD->characterOverlay)
		{
			characterOverlay = chaosHUD->characterOverlay;

			if (characterOverlay)
			{
				setHUDHealth(HUDHealth, HUDMaxHealth); //show cached values
				setHUDScore(HUDScore);
				setHUDDefeats(HUDDefeats); 
			}
		}
	}

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

void AChaosPlayerController::onMatchStateSet(FName state)
{
	matchState = state;

	if (matchState == MatchState::InProgress)
	{
		handleMatchHasStarted();
	}
}

void AChaosPlayerController::handleMatchHasStarted()
{
	chaosHUD = chaosHUD == nullptr ? Cast<AChaosHUD>(GetHUD()) : chaosHUD;

	if (chaosHUD)
	{
		chaosHUD->addCharacterOverlay(); //will only show HUD when in correct match state

		if (chaosHUD->announcement) //if valid, then remove from viewport (as warm up time is over)
		{
			chaosHUD->announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AChaosPlayerController::OnRep_matchState()
{
	if (matchState == MatchState::InProgress)
	{
		handleMatchHasStarted();
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
	else
	{
		bIntializeCharacterOverlay = true;
		HUDHealth = health; //cache in case not intializing properly
		HUDMaxHealth = maxHealth;
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
	else
	{
		bIntializeCharacterOverlay = true;
		HUDScore = score;
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
	else
	{
		bIntializeCharacterOverlay = true;
		HUDDefeats = defeats;
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

void AChaosPlayerController::setHUDAnnouncementCountdown(float countdownTime)
{
	chaosHUD = chaosHUD == nullptr ? Cast<AChaosHUD>(GetHUD()) : chaosHUD;

	bool bHUDValid = chaosHUD &&
		chaosHUD->announcement &&
		chaosHUD->announcement->WarmupTime;

	if (bHUDValid)
	{
		int32 minutes = FMath::FloorToInt(countdownTime / 60.f); //this will get minutes
		int32 seconds = countdownTime - minutes * 60; //gets seconds

		FString countdownText = FString::Printf(TEXT("%02d:%02d"), minutes, seconds); //will show time with correct formatting (10:03), for example
		chaosHUD->announcement->WarmupTime->SetText(FText::FromString(countdownText));
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
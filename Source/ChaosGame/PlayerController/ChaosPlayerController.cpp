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

void AChaosPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AChaosCharacter* chaosCharacter = Cast<AChaosCharacter>(InPawn); //cast to pawn

	if (chaosCharacter)
	{
		setHUDHealth(chaosCharacter->getHealth(), chaosCharacter->getMaxHealth());
	}


}

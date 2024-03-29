// Fill out your copyright notice in the Description page of Project Settings.


#include "ChaosHUD.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlay.h"
#include "ChaosGame/HUD/Announcement.h"

void AChaosHUD::BeginPlay()
{
	Super::BeginPlay();

}

void AChaosHUD::addCharacterOverlay()
{
	APlayerController* playerController = GetOwningPlayerController();

	if (playerController && characterOverlayClass)
	{
		characterOverlay = CreateWidget<UCharacterOverlay>(playerController, characterOverlayClass);
		characterOverlay->AddToViewport();
	}
}

void AChaosHUD::addAnnouncement()
{
	APlayerController* playerController = GetOwningPlayerController();

	if (playerController && announcementClass && announcement == nullptr)
	{
		announcement = CreateWidget<UAnnouncement>(playerController, announcementClass);
		announcement->AddToViewport();
	}
}


void AChaosHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D viewPortSize;

	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(viewPortSize);
		const FVector2D viewportCenter(viewPortSize.X / 2.f, viewPortSize.Y / 2.f); //getting center to draw crosshairs

		float spreadScaled = crosshairSpreadMax * HUDPackage.crosshairSpread;

		if (HUDPackage.crosshairsCenter)
		{
			FVector2D spread(0.f, 0.f); //no spread since center crosshair
			drawCrosshair(HUDPackage.crosshairsCenter, viewportCenter, spread, HUDPackage.crosshairsColor); //use function 
		}
		if (HUDPackage.crosshairsLeft)
		{
			FVector2D spread(-spreadScaled, 0.f); //will move based on char movement (aiming, not aiming etc,)
			drawCrosshair(HUDPackage.crosshairsLeft, viewportCenter, spread, HUDPackage.crosshairsColor);
		}
		if (HUDPackage.crosshairsRight)
		{
			FVector2D spread(spreadScaled, 0.f);
			drawCrosshair(HUDPackage.crosshairsRight, viewportCenter, spread, HUDPackage.crosshairsColor);
		}
		if (HUDPackage.crosshairsBottom)
		{
			FVector2D spread(0.f, spreadScaled); 
			drawCrosshair(HUDPackage.crosshairsBottom, viewportCenter, spread, HUDPackage.crosshairsColor);
		}
		if (HUDPackage.crosshairsTop)
		{
			FVector2D spread(0.f, -spreadScaled); //remember for UV coords, -ve is upwards
			drawCrosshair(HUDPackage.crosshairsTop, viewportCenter, spread, HUDPackage.crosshairsColor);
		}
	}

}

void AChaosHUD::drawCrosshair(UTexture2D* texture, FVector2D viewportCenter, FVector2D spread, FLinearColor crosshairsColor)
{
	const float textureWidth = texture->GetSizeX();
	const float textureHeight = texture->GetSizeY(); //need to do some math to get perfect center position

	const FVector2D textureDrawPoint(
		viewportCenter.X - (textureWidth / 2.f) + spread.X,
		viewportCenter.Y - (textureHeight / 2.f) + spread.Y
	);


	DrawTexture( //HUD function to draw crosshair using the math done
		texture,
		textureDrawPoint.X,
		textureDrawPoint.Y,
		textureWidth,
		textureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		crosshairsColor
	);
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "ChaosHUD.h"

void AChaosHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D viewPortSize;

	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(viewPortSize);
		const FVector2D viewportCenter(viewPortSize.X / 2.f, viewPortSize.Y / 2.f); //getting center to draw crosshairs

		if (HUDPackage.crosshairsCenter)
		{
			drawCrosshair(HUDPackage.crosshairsCenter, viewportCenter); //use function 
		}
		if (HUDPackage.crosshairsLeft)
		{
			drawCrosshair(HUDPackage.crosshairsLeft, viewportCenter);
		}
		if (HUDPackage.crosshairsRight)
		{
			drawCrosshair(HUDPackage.crosshairsRight, viewportCenter);
		}
		if (HUDPackage.crosshairsBottom)
		{
			drawCrosshair(HUDPackage.crosshairsBottom, viewportCenter);
		}
		if (HUDPackage.crosshairsTop)
		{
			drawCrosshair(HUDPackage.crosshairsTop, viewportCenter);
		}
	}

}

void AChaosHUD::drawCrosshair(UTexture2D* texture, FVector2D viewportCenter)
{
	const float textureWidth = texture->GetSizeX();
	const float textureHeight = texture->GetSizeY(); //need to do some math to get perfect center position

	const FVector2D textureDrawPoint(
		viewportCenter.X - (textureWidth / 2.f),
		viewportCenter.Y - (textureHeight / 2.f)
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
		FLinearColor::White
	);
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ChaosHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage //making a struct so its easy to handle HUD stuff
{
	GENERATED_BODY() //so reflection system knows

public:

	class UTexture2D* crosshairsCenter;
	UTexture2D* crosshairsLeft;
	UTexture2D* crosshairsRight;
	UTexture2D* crosshairsBottom;
	UTexture2D* crosshairsTop;

	float crosshairSpread; //for dynamic spreading of crosshair

};


/**
 * 
 */
UCLASS()
class CHAOSGAME_API AChaosHUD : public AHUD
{
	GENERATED_BODY()
	

public:
	virtual void DrawHUD() override; //overriding built in HUD function

private:
	FHUDPackage HUDPackage;

	void drawCrosshair(UTexture2D* texture, FVector2D viewportCenter, FVector2D spread); //handy function to make the crosshairs

	UPROPERTY(EditAnywhere)
	float crosshairSpreadMax = 16.f;

public:

	FORCEINLINE void SetHUDPackage(const FHUDPackage& package) { HUDPackage = package; }
};

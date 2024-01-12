// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

UCLASS()
class CHAOSGAME_API ACasing : public AActor
{
	GENERATED_BODY()
	
public:	
	ACasing();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit); //override

private:

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* casingMesh;

	UPROPERTY(EditAnywhere)
	float shellEjectionImpulse; //how hard it launches

	UPROPERTY(EditAnywhere)
	class USoundCue* shellSound;


public:	
	

	

};

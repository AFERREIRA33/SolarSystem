// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Planet.h"
#include "PlanetGravity.generated.h"




UCLASS()
class SOLARSYSTEM_API APlanetGravity : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APlanetGravity();
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float G = 0.006;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	APlanet* P1;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	APlanet* P2;

	// UPROPERTY(EditAnywhere,BlueprintReadWrite)
	// TArray<APlanet*> planets;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	
private:

	float CalcForce(float mass1,float mass2,float distance);
	FVector accel;

	


};

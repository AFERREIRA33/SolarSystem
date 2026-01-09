// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetGravity.h"

#include "VectorTypes.h"


// Sets default values
APlanetGravity::APlanetGravity()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APlanetGravity::BeginPlay()
{
	Super::BeginPlay();
	
	FVector direction = P2->GetActorLocation() - P1->GetActorLocation();
	float dist = direction.Size();
	
	float orbitalSpeed = FMath::Sqrt(G * P2->mass / dist);
	
	FVector tangent = FVector::CrossProduct(direction.GetSafeNormal(), FVector::UpVector);
	P1->velocity = tangent * orbitalSpeed;
	
}



float APlanetGravity::CalcForce(float mass1, float mass2, float distance)
{
	return G * (mass1 * mass2) / FMath::Square(distance);
}


// Called every frame
void APlanetGravity::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector direction = P2->GetActorLocation() - P1->GetActorLocation();
	float dist = direction.Size();
	float F = CalcForce(P1->mass,P2->mass,dist);

	FVector fdir =  direction.GetSafeNormal();

	accel = (F / P1->mass) * fdir;
	P1->velocity = P1->velocity + accel * DeltaTime;
	P1->SetActorLocation(P1->GetActorLocation() + P1->velocity * DeltaTime);
}


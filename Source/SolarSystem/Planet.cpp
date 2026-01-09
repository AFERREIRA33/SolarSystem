#include "Planet.h"
#include "Components/StaticMeshComponent.h"



APlanet::APlanet()
{

	PrimaryActorTick.bCanEverTick = true;
	mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InstancedMesh"));
	SetRootComponent(mesh);
	mesh->SetMobility(EComponentMobility::Static);
	mesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
}


void APlanet::BeginPlay()
{
	Super::BeginPlay();
	mass = PlanetMass(GetPlanetRadius());
}

// Called every frame
void APlanet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float APlanet::GetPlanetRadius()
{
	return mesh->Bounds.SphereRadius *100;
}

float APlanet::PlanetMass(float radius)
{
	return gravity * FMath::Square(radius) / G;
}


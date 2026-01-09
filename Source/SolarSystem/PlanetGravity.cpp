#include "PlanetGravity.h"

APlanetGravity::APlanetGravity()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APlanetGravity::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &APlanetGravity::InitializeVelocity);
}

float APlanetGravity::CalcForce(float mass1, float mass2, float distance)
{
	return G * (mass1 * mass2) / FMath::Square(distance);
}

void APlanetGravity::InitializeVelocity()
{
	TArray<APlanet*> sortedPlanets = planets;
	sortedPlanets.Sort([](const APlanet& A, const APlanet& B) {
		return A.gravity > B.gravity;
	});

	//APlanet* centralBody = sortedPlanets[0];

	for (int32 i = 1; i < sortedPlanets.Num(); i++)
	{
		APlanet* planet = sortedPlanets[i];


		APlanet* parent = nullptr;
		float minDist = FLT_MAX;

		for (int32 j = 0; j < i; j++)
		{
			APlanet* candidate = sortedPlanets[j];
			float dist = FVector::Dist(planet->GetActorLocation(), candidate->GetActorLocation());
			if (dist < minDist)
			{
				minDist = dist;
				parent = candidate;
			}
		}

		if (!parent) continue;


		FVector direction = parent->GetActorLocation() - planet->GetActorLocation();
		float dist = direction.Size();
		FVector fdir = direction.GetSafeNormal();


		float orbitalSpeed = FMath::Sqrt(G * parent->mass / dist);

		FVector tangent = FVector::CrossProduct(fdir, FVector::UpVector).GetSafeNormal();
        

		planet->velocity = parent->velocity + tangent * orbitalSpeed;
	}
}

void APlanetGravity::ApplyGravityBetween(APlanet* P1, APlanet* P2, float DeltaTime)
{
	FVector direction = P2->GetActorLocation() - P1->GetActorLocation();
	float dist = direction.Size();

	if (dist < 1.0f) return;

	float F = CalcForce(P1->mass, P2->mass, dist);
	FVector fdir = direction.GetSafeNormal();


	P1->velocity += (F / P1->mass) * fdir * DeltaTime;
	P2->velocity += (F / P2->mass) * (-fdir) * DeltaTime;
}

void APlanetGravity::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (int32 i = 0; i < planets.Num(); i++)
	{
		for (int32 j = i + 1; j < planets.Num(); j++)
		{
			ApplyGravityBetween(planets[i], planets[j], DeltaTime);
		}
	}

	for (APlanet* planet : planets)
	{
		planet->SetActorLocation(planet->GetActorLocation() + planet->velocity * DeltaTime);
	}
}
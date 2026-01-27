#include "PlanetGravity.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
APlanetGravity::APlanetGravity()
{
	PrimaryActorTick.bCanEverTick = true;

	// Sun Mesh
	SunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SunMesh"));
	SetRootComponent(SunMesh);
	SunMesh->SetMobility(EComponentMobility::Static);
	SunMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
}


void APlanetGravity::BeginPlay()
{
	Super::BeginPlay();
	
	Mass = CalculateMass(GetSunRadius());
	
	SpawnPlanets();

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &APlanetGravity::InitializeVelocity);
	
	// Draw orbits after a short delay to ensure velocities are initialized
	if (bShowOrbits)
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &APlanetGravity::CalculateAndDrawOrbits, 0.1f, false);
	}
}

float APlanetGravity::GetSunRadius()
{
	if (SunMesh)
	{
		return SunMesh->Bounds.SphereRadius * 100;
	}
	return 100.0f;
}

float APlanetGravity::CalculateMass(float radius)
{
	return Gravity * FMath::Square(radius) / G;
}

float APlanetGravity::CalcForce(float mass1, float mass2, float distance)
{
	return G * (mass1 * mass2) / FMath::Square(distance);
}

void APlanetGravity::InitializeVelocity()
{
	if (Planets.Num() < 1) return;
	
	SunLocation = GetActorLocation();

	// Set initial velocity for each planet for circular orbit
	for (APlanet* Planet : Planets)
	{
		if (!Planet) continue;

		FVector direction = SunLocation - Planet->GetActorLocation();
		float dist = direction.Size();

		if (dist < KINDA_SMALL_NUMBER) continue;

		float F = CalcForce(Planet->mass, Mass, dist);
		FVector fdir = direction.GetSafeNormal();

		FVector accelInit = (F / Planet->mass) * fdir;
		
		FVector tangent = FVector::CrossProduct(fdir, FVector::UpVector).GetSafeNormal();
		float orbitalSpeed = FMath::Sqrt(accelInit.Size() * dist);

		Planet->velocity = tangent * orbitalSpeed;
	}
}

void APlanetGravity::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	SunLocation = GetActorLocation();

	// For each planet, calculate gravitational acceleration from sun and other planets
	for (APlanet* Planet : Planets)
	{
		if (!Planet) continue;

		FVector totalAccel = FVector::ZeroVector;

		// Calculate attraction from the sun to the planet
		FVector directionToSun = SunLocation - Planet->GetActorLocation();
		float distToSun = directionToSun.Size();

		if (distToSun > KINDA_SMALL_NUMBER)
		{
			float F = CalcForce(Planet->mass, Mass, distToSun);
			FVector fdir = directionToSun.GetSafeNormal();
			totalAccel += (F / Planet->mass) * fdir;
		}

		// Calculate attraction from other planets
		for (APlanet* OtherPlanet : Planets)
		{
			if (!OtherPlanet || OtherPlanet == Planet) continue;

			FVector direction = OtherPlanet->GetActorLocation() - Planet->GetActorLocation();
			float dist = direction.Size();

			if (dist < KINDA_SMALL_NUMBER) continue;

			// Limit atraction to avoid extreme forces at close distances
			float minDist = (Planet->GetPlanetRadius() + OtherPlanet->GetPlanetRadius()) * 2.0f;
			float effectiveDist = FMath::Max(dist, minDist);

			float F = CalcForce(Planet->mass, OtherPlanet->mass, effectiveDist);
			FVector fdir = direction.GetSafeNormal();

			totalAccel += (F / Planet->mass) * fdir;
		}

		// Update velocity with global acceleration
		Planet->velocity = Planet->velocity + totalAccel * DeltaTime;
	}

	// Update planet positions
	for (APlanet* Planet : Planets)
	{
		if (!Planet) continue;
		Planet->SetActorLocation(Planet->GetActorLocation() + Planet->velocity * DeltaTime);
	}
}

void APlanetGravity::CalculateAndDrawOrbits()
{
	if (Planets.Num() < 1) return;
	
	SunLocation = GetActorLocation();

	// For each planet, simulate orbit path and draw it
	for (APlanet* Planet : Planets)
	{
		if (!Planet) continue;

		// Use true position and velocity as starting point
		FVector startPos = Planet->GetActorLocation();
		FVector simPos = startPos;
		FVector simVel = Planet->velocity;

		TArray<FVector> orbitPoints;
		orbitPoints.Add(simPos);

		// Calculate approximate orbital period to set time step
		float orbitRadius = FVector::Dist(startPos, SunLocation);
		float orbitalSpeed = simVel.Size();
		float estimatedPeriod = (2.0f * PI * orbitRadius) / FMath::Max(orbitalSpeed, 1.0f);
		
		float simDeltaTime = estimatedPeriod / (float)OrbitResolution;
		int32 maxIterations = OrbitResolution * 2;

		bool orbitComplete = false;

		for (int32 i = 0; i < maxIterations && !orbitComplete; i++)
		{
			FVector direction = SunLocation - simPos;
			float dist = direction.Size();

			if (dist < KINDA_SMALL_NUMBER) break;

			float F = CalcForce(Planet->mass, Mass, dist);
			FVector fdir = direction.GetSafeNormal();
			FVector accel = (F / Planet->mass) * fdir;
			
			simVel = simVel + accel * simDeltaTime;
			simPos = simPos + simVel * simDeltaTime;

			orbitPoints.Add(simPos);

			// Check if we have completed the orbit by returning close to the start position
			if (i > OrbitResolution / 4)
			{
				float distToStart = FVector::Dist(simPos, startPos);
				if (distToStart < orbitRadius * 0.02f)
				{
					orbitComplete = true;
				}
			}
		}
		
		orbitPoints.Add(startPos);
		
		PreCalculatedOrbits.Add(Planet, orbitPoints);

		// Draw orbit with persistant lines trace
		for (int32 i = 0; i < orbitPoints.Num() - 1; i++)
		{
			DrawDebugLine(GetWorld(),orbitPoints[i],orbitPoints[i + 1],FColor::Red,true,-1.0f,0,OrbitLineThickness);
		}
	}
}

void APlanetGravity::SpawnPlanets()
{
	if (!PlanetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlanetClass is not set! Cannot spawn planets."));
		return;
	}
	
	Planets.Empty();

	SunLocation = GetActorLocation();
	TArray<FVector> SpawnedPositions;
	TArray<float> SpawnedRadii;

	for (int32 i = 0; i < NumberOfPlanets; i++)
	{
		// Random value for planet
		float RandomScale = FMath::RandRange(MinPlanetScale, MaxPlanetScale);
		float RandomGravity = FMath::RandRange(MinPlanetGravity, MaxPlanetGravity);

		FVector SpawnLocation;
		bool bValidPosition = false;
		int32 MaxAttempts = 100;
		int32 Attempts = 0;

		// Find valid spawn position
		while (!bValidPosition && Attempts < MaxAttempts)
		{
			Attempts++;
			
			float OrbitRadius = FMath::RandRange(MinOrbitRadius, MaxOrbitRadius);
			
			float Angle = FMath::RandRange(0.0f, 2.0f * PI);
			
			SpawnLocation = SunLocation + FVector(
				FMath::Cos(Angle) * OrbitRadius,
				FMath::Sin(Angle) * OrbitRadius,
				0.0f
			);

			// Check if position is not too close to existing planets
			bValidPosition = true;
			for (int32 j = 0; j < SpawnedPositions.Num(); j++)
			{
				float DistToExisting = FVector::Dist(SpawnLocation, SpawnedPositions[j]);
				float RequiredDist = MinDistanceBetweenPlanets + SpawnedRadii[j] * 100.0f + RandomScale * 100.0f;
				if (DistToExisting < RequiredDist)
				{
					bValidPosition = false;
					break;
				}
			}
		}

		if (!bValidPosition) continue;

		// Spawn Planet
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		APlanet* NewPlanet = GetWorld()->SpawnActor<APlanet>(PlanetClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

		if (NewPlanet)
		{
			// Initialise with random scale and gravity
			NewPlanet->InitializePlanet(RandomScale, RandomGravity);

			// Add to array
			Planets.Add(NewPlanet);
			SpawnedPositions.Add(SpawnLocation);
			SpawnedRadii.Add(RandomScale);
		}
	}
	
}


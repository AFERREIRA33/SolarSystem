#include "PlanetGravity.h"
#include "DrawDebugHelpers.h"

// Sets default values
APlanetGravity::APlanetGravity()
{

	PrimaryActorTick.bCanEverTick = true;
}


void APlanetGravity::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &APlanetGravity::InitializeVelocity);
	
	// Calculer et dessiner les orbites au prochain tick (après InitializeVelocity)
	if (bShowOrbits)
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &APlanetGravity::CalculateAndDrawOrbits, 0.1f, false);
	}
}

float APlanetGravity::CalcForce(float mass1, float mass2, float distance)
{
	return G * (mass1 * mass2) / FMath::Square(distance);
}

void APlanetGravity::InitializeVelocity()
{
	if (Planets.Num() < 2) return;

	// Trouver la planète la plus massive (le soleil)
	APlanet* Sun = nullptr;
	float maxMass = 0.0f;
	for (APlanet* Planet : Planets)
	{
		if (Planet && Planet->mass > maxMass)
		{
			maxMass = Planet->mass;
			Sun = Planet;
		}
	}

	if (!Sun) return;

	// Initialiser la vitesse orbitale de chaque planète autour du soleil
	for (APlanet* Planet : Planets)
	{
		if (!Planet || Planet == Sun) continue;

		FVector direction = Sun->GetActorLocation() - Planet->GetActorLocation();
		float dist = direction.Size();

		if (dist < KINDA_SMALL_NUMBER) continue;

		float F = CalcForce(Planet->mass, Sun->mass, dist);
		FVector fdir = direction.GetSafeNormal();

		FVector accelInit = (F / Planet->mass) * fdir;

		// Calcul du vecteur tangent pour l'orbite
		FVector tangent = FVector::CrossProduct(fdir, FVector::UpVector).GetSafeNormal();
		float orbitalSpeed = FMath::Sqrt(accelInit.Size() * dist);

		Planet->velocity = tangent * orbitalSpeed;
	}

	// Le soleil reste immobile (ou on peut lui donner une vélocité nulle)
	Sun->velocity = FVector::ZeroVector;
}


// Called every frame
void APlanetGravity::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Trouver le soleil (la planète la plus massive) pour l'exclure du mouvement
	APlanet* Sun = nullptr;
	float maxMass = 0.0f;
	for (APlanet* Planet : Planets)
	{
		if (Planet && Planet->mass > maxMass)
		{
			maxMass = Planet->mass;
			Sun = Planet;
		}
	}

	// Pour chaque planète, calculer la force gravitationnelle totale de toutes les autres
	for (APlanet* Planet : Planets)
	{
		// Le soleil ne bouge pas - il est le centre du système
		if (!Planet || Planet == Sun) continue;

		FVector totalAccel = FVector::ZeroVector;

		// Calculer l'attraction de chaque autre planète
		for (APlanet* OtherPlanet : Planets)
		{
			if (!OtherPlanet || OtherPlanet == Planet) continue;

			FVector direction = OtherPlanet->GetActorLocation() - Planet->GetActorLocation();
			float dist = direction.Size();

			if (dist < KINDA_SMALL_NUMBER) continue; // Éviter division par zéro

			// Pour le soleil, appliquer la gravité normalement
			// Pour les autres planètes, limiter l'attraction à courte distance
			float effectiveDist = dist;
			if (OtherPlanet != Sun)
			{
				// Distance minimale = somme des rayons des deux planètes * 2
				float minDist = (Planet->GetPlanetRadius() + OtherPlanet->GetPlanetRadius()) * 2.0f;
				effectiveDist = FMath::Max(dist, minDist);
			}

			float F = CalcForce(Planet->mass, OtherPlanet->mass, effectiveDist);
			FVector fdir = direction.GetSafeNormal();

			// Ajouter l'accélération due à cette planète
			totalAccel += (F / Planet->mass) * fdir;
		}

		// Mettre à jour la vélocité avec l'accélération totale
		Planet->velocity = Planet->velocity + totalAccel * DeltaTime;
	}

	// Mettre à jour les positions de toutes les planètes (sauf le soleil)
	for (APlanet* Planet : Planets)
	{
		if (!Planet || Planet == Sun) continue;
		Planet->SetActorLocation(Planet->GetActorLocation() + Planet->velocity * DeltaTime);
	}
}

FVector APlanetGravity::CalculateOrbitalVelocity(APlanet* Planet, APlanet* Sun)
{
	FVector direction = Sun->GetActorLocation() - Planet->GetActorLocation();
	float dist = direction.Size();

	if (dist < KINDA_SMALL_NUMBER) return FVector::ZeroVector;

	float F = CalcForce(Planet->mass, Sun->mass, dist);
	FVector fdir = direction.GetSafeNormal();
	FVector accelInit = (F / Planet->mass) * fdir;

	FVector tangent = FVector::CrossProduct(fdir, FVector::UpVector).GetSafeNormal();
	float orbitalSpeed = FMath::Sqrt(accelInit.Size() * dist);

	return tangent * orbitalSpeed;
}

void APlanetGravity::CalculateAndDrawOrbits()
{
	if (Planets.Num() < 2) return;

	// Trouver le soleil (la planète la plus massive)
	APlanet* Sun = nullptr;
	float maxMass = 0.0f;
	for (APlanet* Planet : Planets)
	{
		if (Planet && Planet->mass > maxMass)
		{
			maxMass = Planet->mass;
			Sun = Planet;
		}
	}

	if (!Sun) return;

	// Pour chaque planète, simuler une orbite complète et dessiner
	for (APlanet* Planet : Planets)
	{
		if (!Planet || Planet == Sun) continue;

		// Utiliser la vraie position et vélocité de la planète
		FVector startPos = Planet->GetActorLocation();
		FVector simPos = startPos;
		FVector simVel = Planet->velocity; // Utiliser la vraie vélocité calculée par InitializeVelocity

		TArray<FVector> orbitPoints;
		orbitPoints.Add(simPos);

		// Calculer la période orbitale approximative pour ajuster le pas de temps
		float orbitRadius = FVector::Dist(startPos, Sun->GetActorLocation());
		float orbitalSpeed = simVel.Size();
		float estimatedPeriod = (2.0f * PI * orbitRadius) / FMath::Max(orbitalSpeed, 1.0f);
		
		// Ajuster le pas de temps en fonction de la période orbitale
		float simDeltaTime = estimatedPeriod / (float)OrbitResolution;
		int32 maxIterations = OrbitResolution * 2; // Sécurité pour éviter boucle infinie

		bool orbitComplete = false;
		FVector prevDirection = (simPos - Sun->GetActorLocation()).GetSafeNormal();

		for (int32 i = 0; i < maxIterations && !orbitComplete; i++)
		{
			// Calculer l'accélération due au soleil uniquement (orbite képlerienne pure)
			FVector direction = Sun->GetActorLocation() - simPos;
			float dist = direction.Size();

			if (dist < KINDA_SMALL_NUMBER) break;

			float F = CalcForce(Planet->mass, Sun->mass, dist);
			FVector fdir = direction.GetSafeNormal();
			FVector accel = (F / Planet->mass) * fdir;

			// Mettre à jour vélocité et position (intégration de Verlet simplifiée)
			simVel = simVel + accel * simDeltaTime;
			simPos = simPos + simVel * simDeltaTime;

			orbitPoints.Add(simPos);

			// Vérifier si on a fait un tour complet
			if (i > OrbitResolution / 4)
			{
				float distToStart = FVector::Dist(simPos, startPos);
				if (distToStart < orbitRadius * 0.02f) // 2% de tolérance
				{
					orbitComplete = true;
				}
			}
		}

		// Fermer l'orbite en connectant le dernier point au premier
		orbitPoints.Add(startPos);

		// Stocker l'orbite
		PreCalculatedOrbits.Add(Planet, orbitPoints);

		// Générer une couleur unique pour chaque planète basée sur son index
		int32 planetIndex = Planets.Find(Planet);
		FColor orbitColor = FColor::MakeRedToGreenColorFromScalar((float)planetIndex / (float)Planets.Num());

		// Dessiner l'orbite avec des lignes persistantes et plus épaisses
		for (int32 i = 0; i < orbitPoints.Num() - 1; i++)
		{
			DrawDebugLine(
				GetWorld(),
				orbitPoints[i],
				orbitPoints[i + 1],
				orbitColor,
				true, // Persistant
				-1.0f,
				0,
				OrbitLineThickness
			);
		}
	}
}


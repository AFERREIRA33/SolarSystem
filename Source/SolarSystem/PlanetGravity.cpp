#include "PlanetGravity.h"

// Sets default values
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


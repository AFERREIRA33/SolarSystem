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

	// Tableau contenant toutes les planètes du système solaire
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TArray<APlanet*> Planets;

	// Paramètres de visualisation des orbites
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit Visualization")
	bool bShowOrbits = true;

	// Nombre de points pour calculer l'orbite complète
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit Visualization")
	int32 OrbitResolution = 360;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit Visualization")
	float OrbitLineThickness = 5.0f;

protected:
	virtual void BeginPlay() override;

private:
	float CalcForce(float mass1,float mass2,float distance);
	void InitializeVelocity();
	void CalculateAndDrawOrbits();
	FVector CalculateOrbitalVelocity(APlanet* Planet, APlanet* Sun);

	// Stockage des orbites pré-calculées pour chaque planète
	TMap<APlanet*, TArray<FVector>> PreCalculatedOrbits;
	
};

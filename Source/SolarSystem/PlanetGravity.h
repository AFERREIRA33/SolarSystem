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

	APlanetGravity();
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Properties")
	float G = 0.006;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Properties")
	float Gravity = 100.0f;
	
	UPROPERTY()
	float Mass;
	
	UPROPERTY()
	TArray<APlanet*> Planets;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Spawning")
	int32 NumberOfPlanets = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Spawning")
	float MinOrbitRadius = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Spawning")
	float MaxOrbitRadius = 50000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Spawning")
	float MinDistanceBetweenPlanets = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Spawning")
	float MinPlanetScale = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Spawning")
	float MaxPlanetScale = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Spawning")
	float MinPlanetGravity = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Spawning")
	float MaxPlanetGravity = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Spawning")
	TSubclassOf<APlanet> PlanetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit Visualization")
	bool bShowOrbits = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit Visualization")
	int32 OrbitResolution = 360;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit Visualization")
	float OrbitLineThickness = 5.0f;
	
	float GetSunRadius();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sun Properties")
	UStaticMeshComponent* SunMesh;

private:
	float CalcForce(float mass1, float mass2, float distance);
	void InitializeVelocity();
	void CalculateAndDrawOrbits();
	float CalculateMass(float radius);
	void SpawnPlanets();

	UPROPERTY()
	FVector SunLocation = GetActorLocation();

	TMap<APlanet*, TArray<FVector>> PreCalculatedOrbits;
	
};

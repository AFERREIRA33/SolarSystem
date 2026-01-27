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

protected:
	virtual void BeginPlay() override;

private:
	float CalcForce(float mass1,float mass2,float distance);
	void InitializeVelocity();
	
};

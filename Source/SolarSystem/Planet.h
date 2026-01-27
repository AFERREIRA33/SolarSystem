#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Planet.generated.h"

UCLASS()
class SOLARSYSTEM_API APlanet : public AActor
{
	GENERATED_BODY()

public:
	APlanet();

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float gravity;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FVector velocity = FVector::ZeroVector;

	UPROPERTY()
	float mass;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float G = 0.006;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	float GetPlanetRadius();
	
	void InitializePlanet(float NewScale, float NewGravity);
	
	void RecalculateMass();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* mesh;

private:
	float PlanetMass(float radius);

};

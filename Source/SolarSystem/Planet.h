#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Planet.generated.h"

UCLASS()
class SOLARSYSTEM_API APlanet : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* mesh;

	

private:
	float PlanetMass(float radius);

};

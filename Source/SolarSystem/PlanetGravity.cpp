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
	FVector direction = P2->GetActorLocation() - P1->GetActorLocation();
	float dist = direction.Size();
    
	float F = CalcForce(P1->mass, P2->mass, dist);
	FVector fdir = direction.GetSafeNormal();
    
	accel = (F / P1->mass) * fdir;
    
	FVector tangent = FVector::CrossProduct(fdir, FVector::UpVector).GetSafeNormal();
	float orbitalSpeed = FMath::Sqrt(accel.Size() * dist);
    
	P1->velocity = tangent * orbitalSpeed;
}


// Called every frame
void APlanetGravity::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector direction = P2->GetActorLocation() - P1->GetActorLocation();
	float dist = direction.Size();
	float F = CalcForce(P1->mass,P2->mass,dist);

	FVector fdir =  direction.GetSafeNormal();

	accel = (F / P1->mass) * fdir;
	P1->velocity = P1->velocity + accel * DeltaTime;
	P1->SetActorLocation(P1->GetActorLocation() + P1->velocity * DeltaTime);
}


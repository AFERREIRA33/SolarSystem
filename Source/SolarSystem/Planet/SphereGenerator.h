#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "SurfaceGenerator/GenerateSurface.h"
#include "Utils/FastNoiseLite.h"
#include "SphereGenerator.generated.h"

UCLASS()
class SOLARSYSTEM_API ASphereGenerator : public AGenerateSurface
{
    GENERATED_BODY()

public:
    ASphereGenerator();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
    float Radius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet", meta = (ClampMin = "0", ClampMax = "6"))
    int32 Subdivisions = 6;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Planet")
    UProceduralMeshComponent* MeshComponent;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
    float HeightAmplitude = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
    float NoiseScale = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
    float gravity = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
    float G = 0.1f;

    UPROPERTY(EditAnywhere,BlueprintReadWrite)
    FVector velocity = FVector::ZeroVector;

    UPROPERTY()
    float Mass;
    
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TMap<int64, int32> MiddlePointCache;
    
    
    virtual ProceduralGenerationType SetGenerationType() override;
    void Setup() override;
    virtual void Generate2DHeightMap(FVector Position) override;
    TArray<float> HeightMap;
    
    
    float GetPlanetRadius();
    float PlanetMass(float radius);
    void InitializePlanet(float NewScale, float NewGravity, TObjectPtr<UMaterialInterface>);
    void RecalculateMass();
    
protected:
    virtual void BeginPlay() override;

private:
    void GenerateIcosphere();
    void SubdivideTriangle(int32 I1, int32 I2, int32 I3, int32 Depth, TMap<int64, int32>& Cache);
    int32 GetMiddlePoint(int32 P1, int32 P2, TMap<int64, int32>& Cache);
    int32 GetOrCreateVertex(FVector Position, TMap<FVector, int32>& VertexCache);
    void CalculateSmoothNormals();
    void StartGeneration() override;
    FVector GetNoisyPosition(const FVector& V);
    FVector2D GetUV(FVector Position, FVector Normal) const;

    // Golden ratio for icosahedron
    const float T = (1.0f + FMath::Sqrt(5.0f)) / 2.0f;

    // 12 vertices of icosahedron
    TArray<FVector> IcoVertices = {
        FVector(-1, T, 0).GetSafeNormal(), FVector(1, T, 0).GetSafeNormal(),
        FVector(-1, -T, 0).GetSafeNormal(), FVector(1, -T, 0).GetSafeNormal(),
        FVector(0, -1, T).GetSafeNormal(), FVector(0, 1, T).GetSafeNormal(),
        FVector(0, -1, -T).GetSafeNormal(), FVector(0, 1, -T).GetSafeNormal(),
        FVector(T, 0, -1).GetSafeNormal(), FVector(T, 0, 1).GetSafeNormal(),
        FVector(-T, 0, -1).GetSafeNormal(), FVector(-T, 0, 1).GetSafeNormal()
    };

    // 20 faces of icosahedron (vertex indices)
    int32 Faces[20][3] = {
        {0,11,5}, {0,5,1}, {0,1,7}, {0,7,10}, {0,10,11},
        {1,5,9}, {5,11,4}, {11,10,2}, {10,7,6}, {7,1,8},
        {3,9,4}, {3,4,2}, {3,2,6}, {3,6,8}, {3,8,9},
        {4,9,5}, {2,4,11}, {6,2,10}, {8,6,7}, {9,8,1}
    };
};
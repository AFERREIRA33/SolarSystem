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
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet", meta = (ClampMin = "0", ClampMax = "6"))
    int32 Subdivisions = 3;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Planet")
    UProceduralMeshComponent* MeshComponent;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
    float HeightAmplitude = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
    float NoiseScale = 1.0f;

    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TMap<int64, int32> MiddlePointCache;


    virtual ProceduralGenerationType SetGenerationType() override;
    void Setup() override;
    virtual void Generate2DHeightMap(FVector Position) override;
    TArray<float> HeightMap;
    
protected:
    virtual void BeginPlay() override;

private:
    void GenerateIcosphere();
    void SubdivideTriangle(const FVector& V1, const FVector& V2, const FVector& V3, int32 Depth);
    int32 GetMiddlePoint(int32 P1, int32 P2, TMap<int64, int32>& Cache);
    void StartGeneration() override;
    void GenerateMesh() override;

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
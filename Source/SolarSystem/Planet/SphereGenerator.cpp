#include "SphereGenerator.h"

ASphereGenerator::ASphereGenerator()
{
    PrimaryActorTick.bCanEverTick = true;
    MeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
}

void ASphereGenerator::BeginPlay()
{
    Super::BeginPlay();
    GenerateIcosphere();
}

void ASphereGenerator::GenerateIcosphere()
{
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TMap<int64, int32> MiddlePointCache;
    
    // Subdivide each face
    for (int32 i = 0; i < 20; i++)
    {
        SubdivideTriangle(IcoVertices[Faces[i][0]], IcoVertices[Faces[i][1]], 
            IcoVertices[Faces[i][2]], Subdivisions);
    }

    // Scale vertices by radius
    for (FVector& V : Vertices)
    {
        V *= Radius;
    }

    // Generate normals and UVs
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    for (const FVector& V : Vertices)
    {
        Normals.Add(V.GetSafeNormal());
        FVector N = V.GetSafeNormal();
        UVs.Add(FVector2D(0.5f + FMath::Atan2(N.Y, N.X) / (2 * PI), 0.5f - FMath::Asin(N.Z) / PI));
    }

    MeshComponent->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, TArray<FColor>(), TArray<FProcMeshTangent>(), true);
}

void ASphereGenerator::SubdivideTriangle(const FVector& V1, const FVector& V2, const FVector& V3, int32 Depth)
{
    if (Depth == 0)
    {
        int32 I1 = Vertices.Add(V1);
        int32 I2 = Vertices.Add(V2);
        int32 I3 = Vertices.Add(V3);
        Triangles.Append({I1, I3, I2});
        return;
    }

    FVector M1 = ((V1 + V2) / 2).GetSafeNormal();
    FVector M2 = ((V2 + V3) / 2).GetSafeNormal();
    FVector M3 = ((V3 + V1) / 2).GetSafeNormal();

    SubdivideTriangle(V1, M1, M3, Depth - 1);
    SubdivideTriangle(V2, M2, M1, Depth - 1);
    SubdivideTriangle(V3, M3, M2, Depth - 1);
    SubdivideTriangle(M1, M2, M3, Depth - 1);
}

int32 ASphereGenerator::GetMiddlePoint(int32 P1, int32 P2, TMap<int64, int32>& Cache)
{
    int64 Key = (int64)FMath::Min(P1, P2) << 32 | FMath::Max(P1, P2);
    if (Cache.Contains(Key)) return Cache[Key];
    
    FVector Middle = ((Vertices[P1] + Vertices[P2]) / 2).GetSafeNormal();
    int32 Index = Vertices.Add(Middle);
    Cache.Add(Key, Index);
    return Index;
}


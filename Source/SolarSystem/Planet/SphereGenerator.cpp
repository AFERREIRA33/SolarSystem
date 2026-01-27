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
}

void ASphereGenerator::GenerateIcosphere()
{
    Vertices.Empty();
    Triangles.Empty();
    TMap<int64, int32> Cache;

    // 1. Initial 12 vertices (displaced by noise)
    for (int32 i = 0; i < 12; i++)
    {
        Vertices.Add(GetNoisyPosition(IcoVertices[i]));
    }

    // 2. Subdivide the 20 initial faces
    for (int32 i = 0; i < 20; i++)
    {
        SubdivideTriangle(Faces[i][0], Faces[i][1], Faces[i][2], Subdivisions, Cache);
    }

    // 3. Generate Smooth Normals and UVs
    TArray<FVector> Normals;
    Normals.SetNumZeroed(Vertices.Num());

    for (int32 i = 0; i < Triangles.Num(); i += 3)
    {
        int32 ID0 = Triangles[i];
        int32 ID1 = Triangles[i + 1];
        int32 ID2 = Triangles[i + 2];

        FVector V0 = Vertices[ID0];
        FVector V1 = Vertices[ID1];
        FVector V2 = Vertices[ID2];

        FVector FaceNormal = FVector::CrossProduct(V2 - V0, V1 - V0).GetSafeNormal();

        Normals[ID0] += FaceNormal;
        Normals[ID1] += FaceNormal;
        Normals[ID2] += FaceNormal;
    }

    TArray<FVector2D> UVs;
    for (int32 i = 0; i < Vertices.Num(); i++)
    {
        Normals[i].Normalize();
        UVs.Add(GetUV(Vertices[i], Normals[i]));
    }
    MeshComponent->SetMaterial(0, Material);
    MeshComponent->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, TArray<FColor>(), TArray<FProcMeshTangent>(), true);

}

int32 ASphereGenerator::GetMiddlePoint(int32 P1, int32 P2, TMap<int64, int32>& Cache)
{
    // Create a unique key for the edge regardless of order
    int64 Key = (int64)FMath::Min(P1, P2) << 32 | FMath::Max(P1, P2);
    
    if (Cache.Contains(Key)) return Cache[Key];
    
    // Calculate normalized midpoint to stay on the sphere unit
    FVector V1 = Vertices[P1];
    FVector V2 = Vertices[P2];
    
    // We assume Vertices[P1] is already at the noisy position, 
    // so we get the "original" sphere position by normalizing the sum
    FVector MiddleSphere = ((V1.GetSafeNormal() + V2.GetSafeNormal()) * 0.5f).GetSafeNormal();
    
    // Apply noise once
    FVector NoisyPos = GetNoisyPosition(MiddleSphere);
    int32 Index = Vertices.Add(NoisyPos);
    
    Cache.Add(Key, Index);
    return Index;
}

void ASphereGenerator::SubdivideTriangle(int32 I1, int32 I2, int32 I3, int32 Depth, TMap<int64, int32>& Cache)
{
    if (Depth == 0)
    {
        Triangles.Append({I1, I3, I2}); // Wound for Unreal's CCW
        return;
    }

    int32 M1 = GetMiddlePoint(I1, I2, Cache);
    int32 M2 = GetMiddlePoint(I2, I3, Cache);
    int32 M3 = GetMiddlePoint(I3, I1, Cache);

    SubdivideTriangle(I1, M1, M3, Depth - 1, Cache);
    SubdivideTriangle(I2, M2, M1, Depth - 1, Cache);
    SubdivideTriangle(I3, M3, M2, Depth - 1, Cache);
    SubdivideTriangle(M1, M2, M3, Depth - 1, Cache);
}

int32 ASphereGenerator::GetOrCreateVertex(FVector Position, TMap<FVector, int32>& VertexCache)
{
    // Round position slightly to avoid floating point precision issues in the Map key
    FVector Key = FVector(FMath::RoundToFloat(Position.X * 100), FMath::RoundToFloat(Position.Y * 100), FMath::RoundToFloat(Position.Z * 100));
    
    if (VertexCache.Contains(Key))
    {
        return VertexCache[Key];
    }

    // Apply noise only once per unique vertex
    FVector NoisyPos = GetNoisyPosition(Position.GetSafeNormal());
    int32 NewIndex = Vertices.Add(NoisyPos);
    VertexCache.Add(Key, NewIndex);
    return NewIndex;
}

void ASphereGenerator::StartGeneration()
{
    Noise->SetSeed(FMath::RandRange(0, 100000));
    Noise->SetFrequency(Frequency);
    Noise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    Noise->SetFractalType(FastNoiseLite::FractalType_FBm);

    GenerationType = SetGenerationType();

    Setup();

    GenerateIcosphere();
    
    //GenerateHeightMap();
    
    UE_LOG(LogTemp, Warning, TEXT("Vertices Count : %d"), Vertices.Num());
    UE_LOG(LogTemp, Warning, TEXT("Vertex Count : %d"), VertexCount);

    //ApplyMesh();
}


void ASphereGenerator::Setup()
{
    int Dim = Size + 1;
    Voxels.SetNumZeroed(Dim * Dim * Dim);
    
    FVector Center = FVector(Size / 2.0f, Size / 2.0f, Size / 2.0f);
    float SphereRadius = Size / 2.5f; // Adjust as needed
    
    for (int x = 0; x < Dim; x++)
    {
        for (int y = 0; y < Dim; y++)
        {
            for (int z = 0; z < Dim; z++)
            {
                FVector Point = FVector(x, y, z);
                float Distance = FVector::Dist(Point, Center);
                
                // Positive inside sphere, negative outside
                float Density = SphereRadius - Distance;
                
                // Optional: Add noise for terrain variation
                // Density += Noise->GetNoise(x * NoiseScale, y * NoiseScale, z * NoiseScale) * HeightAmplitude;
                
                int Index = x + y * Dim + z * Dim * Dim;
                Voxels[Index] = Density;
            }
        }
    }
}
ProceduralGenerationType ASphereGenerator::SetGenerationType()
{
    return ProceduralGenerationType::GT_2D;
}

void ASphereGenerator::Generate2DHeightMap(FVector Position)
{
    for (int32 i = 0; i < Vertices.Num(); ++i)
    {
        FVector VertexPos = Vertices[i] * Radius;
        float NoiseValue = Noise->GetNoise(VertexPos.X * NoiseScale, VertexPos.Y * NoiseScale, VertexPos.Z * NoiseScale);
        NoiseValue = (NoiseValue + 1.0f) * 0.5f; // Normalize to 0-1
        
        // Displace vertex along its normal direction
        Vertices[i] = (Vertices[i] * Radius + Vertices[i] * NoiseValue * HeightAmplitude) / Radius;
    }
}

FVector ASphereGenerator::GetNoisyPosition(const FVector& UnitDir)
{
    // UnitDir must be a normalized vector (length 1.0)
    FVector P = UnitDir * Radius;
    float NoiseValue = Noise->GetNoise(P.X * NoiseScale, P.Y * NoiseScale, P.Z * NoiseScale);
    
    // Map noise from [-1, 1] to [0, 1] then apply amplitude
    float Displacement = (NoiseValue + 1.0f) * 0.5f * HeightAmplitude;
    
    return UnitDir * (Radius + Displacement);
}

void ASphereGenerator::CalculateSmoothNormals()
{
    TArray<FVector> Normals;
    Normals.SetNumZeroed(Vertices.Num());

    // Sum up face normals for every triangle
    for (int32 i = 0; i < Triangles.Num(); i += 3)
    {
        int32 ID0 = Triangles[i];
        int32 ID1 = Triangles[i+1];
        int32 ID2 = Triangles[i+2];

        FVector V0 = Vertices[ID0];
        FVector V1 = Vertices[ID1];
        FVector V2 = Vertices[ID2];

        FVector FaceNormal = FVector::CrossProduct(V1 - V0, V2 - V0).GetSafeNormal();

        Normals[ID0] += FaceNormal;
        Normals[ID1] += FaceNormal;
        Normals[ID2] += FaceNormal;
    }

    // Normalize the results
    for (FVector& N : Normals)
    {
        N.Normalize();
    }
    
    // Pass these Normals to MeshComponent->CreateMeshSection
}

FVector2D ASphereGenerator::GetUV(FVector Position, FVector Normal) const
{
    // Assume sphere center is at (0,0,0)
    float Distance = Position.Size(); // Distance from center
    float V = FMath::Clamp((Distance - Radius) / HeightAmplitude, 0.0f, 1.0f);

    // U can be based on angle or just set to 0 for a simple radial gradient
    float U = 0.0f;

    return FVector2D(U, abs(V));
}

float ASphereGenerator::GetPlanetRadius()
{
	return Radius;
}

float ASphereGenerator::PlanetMass(float radius)
{
	return gravity * FMath::Square(radius) / G;
}

void ASphereGenerator::InitializePlanet(float NewScale, float NewGravity, TObjectPtr<UMaterialInterface> NewMaterial)
{
    Radius *= NewScale;
    Material = NewMaterial;
    StartGeneration();
	gravity = NewGravity;
	
	RecalculateMass();
}

void ASphereGenerator::RecalculateMass()
{
    
    Mass = PlanetMass(GetPlanetRadius());
}


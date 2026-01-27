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
    UE_LOG(LogTemp, Warning, TEXT("SphereGenerator BeginPlay"));
    StartGeneration();
}

void ASphereGenerator::GenerateIcosphere()
{
    
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

void ASphereGenerator::StartGeneration()
{
    Noise->SetSeed(FMath::RandRange(0, 100000));
    Noise->SetFrequency(Frequency);
    Noise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    Noise->SetFractalType(FastNoiseLite::FractalType_FBm);

    GenerationType = SetGenerationType();

    Setup();

    GenerateIcosphere();
    
    GenerateHeightMap();
	
    GenerateMesh();
    UE_LOG(LogTemp, Warning, TEXT("Vertices Count : %d"), Vertices.Num());
    UE_LOG(LogTemp, Warning, TEXT("Vertex Count : %d"), VertexCount);

    ApplyMesh();
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

void ASphereGenerator::GenerateMesh()
{
    VertexCount = Vertices.Num();
    
    // Copy vertices and triangles to MeshData
    MeshData.Vertices = Vertices;
    MeshData.Triangles = Triangles;

    // Generate normals (for a sphere, normals point outward from center)
    MeshData.Normals.SetNum(Vertices.Num());
    for (int32 i = 0; i < Vertices.Num(); i++)
    {
        MeshData.Normals[i] = Vertices[i].GetSafeNormal();
    }

    // Generate UVs using spherical mapping
    MeshData.UV0.SetNum(Vertices.Num());
    for (int32 i = 0; i < Vertices.Num(); i++)
    {
        FVector Normal = Vertices[i].GetSafeNormal();
        float U = 0.5f + FMath::Atan2(Normal.Y, Normal.X) / (2.0f * PI);
        float V = 0.5f - FMath::Asin(FMath::Clamp(Normal.Z, -1.0f, 1.0f)) / PI;
        MeshData.UV0[i] = FVector2D(U, V);
    }

    // Initialize colors
    MeshData.Colors.SetNum(Vertices.Num());
    for (int32 i = 0; i < Vertices.Num(); i++)
    {
        MeshData.Colors[i] = FColor::White;
    }

    VertexCount = Vertices.Num();
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


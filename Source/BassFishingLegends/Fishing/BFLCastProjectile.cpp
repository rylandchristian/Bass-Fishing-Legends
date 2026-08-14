#include "Fishing/BFLCastProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Fishing/BFLFishingComponent.h"
#include "Game/BFLAssignedMesh.h"
#include "Game/BFLStatics.h"

ABFLCastProjectile::ABFLCastProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(8.f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = Collision;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetRelativeScale3D(FVector(0.12f));
}

void ABFLCastProjectile::BeginPlay()
{
	Super::BeginPlay();

	UStaticMesh* const MeshBefore = Mesh ? Mesh->GetStaticMesh() : nullptr;
	if (UStaticMesh* Sphere = UBFLStatics::GetEngineMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere")))
	{
		if (Mesh)
		{
			UStaticMesh* Current = Mesh->GetStaticMesh();
			Mesh->SetStaticMesh(BFLAssignedMesh::Keep(Current, Sphere));
		}
	}
	if (!MeshBefore && Mesh)
	{
		if (UMaterialInstanceDynamic* Mat = UBFLStatics::MakeTintedMeshMaterial(this, FLinearColor(0.95f, 0.75f, 0.15f)))
		{
			Mesh->SetMaterial(0, Mat);
		}
	}
}

void ABFLCastProjectile::Launch(const FVector& InVelocity, float InWaterZ, UBFLFishingComponent* InOwnerFishing)
{
	Velocity = InVelocity;
	WaterZ = InWaterZ;
	OwnerFishing = InOwnerFishing;
	Age = 0.f;
	bHasHit = false;
}

void ABFLCastProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bHasHit)
	{
		return;
	}

	Age += DeltaSeconds;
	if (Age >= MaxLifetime)
	{
		if (UBFLFishingComponent* Fishing = OwnerFishing.Get())
		{
			Fishing->NotifyCastMissed();
		}
		Destroy();
		return;
	}

	const FVector Start = GetActorLocation();
	Velocity.Z += GravityZ * DeltaSeconds;
	const FVector End = Start + Velocity * DeltaSeconds;

	if (Start.Z > WaterZ && End.Z <= WaterZ)
	{
		const float Alpha = (Start.Z - WaterZ) / FMath::Max(Start.Z - End.Z, KINDA_SMALL_NUMBER);
		FVector Hit = FMath::Lerp(Start, End, Alpha);
		Hit.Z = WaterZ;
		HitWater(Hit);
		return;
	}

	SetActorLocation(End);
}

void ABFLCastProjectile::HitWater(const FVector& HitLocation)
{
	bHasHit = true;
	SetActorLocation(HitLocation);

	if (UBFLFishingComponent* Fishing = OwnerFishing.Get())
	{
		Fishing->NotifyCastHitWater(HitLocation);
	}

	Destroy();
}

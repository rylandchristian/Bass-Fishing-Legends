#include "Fishing/BFLBaitActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Fish/BFLFishActor.h"
#include "Game/BFLAssignedMesh.h"
#include "Game/BFLGameMode.h"
#include "Game/BFLStatics.h"

ABFLBaitActor::ABFLBaitActor()
{
	PrimaryActorTick.bCanEverTick = true;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(18.f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Collision->SetGenerateOverlapEvents(true);
	RootComponent = Collision;

	BobberMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bobber"));
	BobberMesh->SetupAttachment(RootComponent);
	BobberMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BobberMesh->SetRelativeScale3D(FVector(0.18f, 0.18f, 0.22f));

	HookMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Hook"));
	HookMesh->SetupAttachment(RootComponent);
	HookMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HookMesh->SetRelativeLocation(FVector(0.f, 0.f, -22.f));
	HookMesh->SetRelativeScale3D(FVector(0.06f, 0.06f, 0.18f));
}

void ABFLBaitActor::BeginPlay()
{
	Super::BeginPlay();

	UStaticMesh* const BobberBefore = BobberMesh ? BobberMesh->GetStaticMesh() : nullptr;
	UStaticMesh* const HookBefore = HookMesh ? HookMesh->GetStaticMesh() : nullptr;
	if (UStaticMesh* Sphere = UBFLStatics::GetEngineMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere")))
	{
		if (BobberMesh)
		{
			UStaticMesh* Current = BobberMesh->GetStaticMesh();
			BobberMesh->SetStaticMesh(BFLAssignedMesh::Keep(Current, Sphere));
		}
	}
	if (UStaticMesh* Cone = UBFLStatics::GetEngineMesh(TEXT("/Engine/BasicShapes/Cone.Cone")))
	{
		if (HookMesh)
		{
			UStaticMesh* Current = HookMesh->GetStaticMesh();
			HookMesh->SetStaticMesh(BFLAssignedMesh::Keep(Current, Cone));
		}
	}
	if (!BobberBefore && BobberMesh)
	{
		if (UMaterialInstanceDynamic* Red = UBFLStatics::MakeTintedMeshMaterial(this, FLinearColor(0.85f, 0.08f, 0.08f)))
		{
			BobberMesh->SetMaterial(0, Red);
		}
	}
	if (!HookBefore && HookMesh)
	{
		if (UMaterialInstanceDynamic* White = UBFLStatics::MakeTintedMeshMaterial(this, FLinearColor(0.9f, 0.9f, 0.85f)))
		{
			HookMesh->SetMaterial(0, White);
		}
	}

	BobPhase = FMath::FRand() * PI * 2.f;

	if (ABFLGameMode* GM = UBFLStatics::GetBFLGameMode(this))
	{
		GM->RegisterBait(this);
	}
}

void ABFLBaitActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ABFLGameMode* GM = UBFLStatics::GetBFLGameMode(this))
	{
		GM->UnregisterBait(this);
	}
	Super::EndPlay(EndPlayReason);
}

void ABFLBaitActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	Age += DeltaSeconds;
	if (Lifetime > 0.f && Age >= Lifetime && IsAvailable())
	{
		Destroy();
		return;
	}

	const float WaterZ = UBFLStatics::GetWaterHeight(this);
	BobPhase += DeltaSeconds * 2.2f;
	const float Bob = FMath::Sin(BobPhase) * 4.f;
	FVector Loc = GetActorLocation();
	Loc.Z = WaterZ + 6.f + Bob;
	SetActorLocation(Loc);
}

void ABFLBaitActor::HookFish(ABFLFishActor* Fish)
{
	HookedFish = Fish;
	bAvailable = false;
}

void ABFLBaitActor::ReleaseHook()
{
	HookedFish = nullptr;
	bAvailable = true;
}

FVector ABFLBaitActor::GetBobberLocation() const
{
	return GetActorLocation();
}

#include "Fish/BFLFishActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Fish/BFLFishCatalog.h"
#include "Fish/BFLFishLook.h"
#include "Fishing/BFLBaitActor.h"
#include "Fishing/BFLFishingComponent.h"
#include "Game/BFLAssignedMesh.h"
#include "Game/BFLGameMode.h"
#include "Game/BFLGameSettings.h"
#include "Game/BFLStatics.h"

ABFLFishActor::ABFLFishActor()
{
	PrimaryActorTick.bCanEverTick = true;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(40.f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetGenerateOverlapEvents(false);
	RootComponent = Collision;

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	BodyMesh->SetupAttachment(RootComponent);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TailMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tail"));
	TailMesh->SetupAttachment(RootComponent);
	TailMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TailMesh->SetRelativeLocation(FVector(-45.f, 0.f, 0.f));
	TailMesh->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	TailMesh->SetRelativeScale3D(FVector(0.35f, 0.08f, 0.45f));

	SpeciesDef = UBFLFishCatalog::MakeBuiltin(EFishSpecies::LargemouthBass);
}

void ABFLFishActor::ApplyPlaceholderMeshes()
{
	if (const UBFLGameSettings* Settings = UBFLGameSettings::Get())
	{
		if (UStaticMesh* Authored = Settings->FishBodyMesh.LoadSynchronous())
		{
			if (BodyMesh)
			{
				UStaticMesh* Current = BodyMesh->GetStaticMesh();
				BodyMesh->SetStaticMesh(BFLAssignedMesh::Keep(Current, Authored));
			}
		}
	}

	bHasAuthoredBodyMesh = BodyMesh && BodyMesh->GetStaticMesh() != nullptr;
	if (!bHasAuthoredBodyMesh)
	{
		if (UStaticMesh* Sphere = UBFLStatics::GetEngineMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere")))
		{
			if (BodyMesh)
			{
				UStaticMesh* Current = BodyMesh->GetStaticMesh();
				BodyMesh->SetStaticMesh(BFLAssignedMesh::Keep(Current, Sphere));
			}
		}
		if (UStaticMesh* Cone = UBFLStatics::GetEngineMesh(TEXT("/Engine/BasicShapes/Cone.Cone")))
		{
			if (TailMesh)
			{
				UStaticMesh* Current = TailMesh->GetStaticMesh();
				TailMesh->SetStaticMesh(BFLAssignedMesh::Keep(Current, Cone));
			}
		}
		return;
	}

	if (TailMesh && !BFLFishLook::ShouldShowTail(bHasAuthoredBodyMesh))
	{
		TailMesh->SetVisibility(false);
		TailMesh->SetHiddenInGame(true);
		TailMesh->SetStaticMesh(nullptr);
	}
}

void ABFLFishActor::BeginPlay()
{
	Super::BeginPlay();

	ApplyPlaceholderMeshes();

	if (WeightLbs <= 0.1f)
	{
		InitializeSpecies(SpeciesDef);
	}
	else
	{
		ApplyVisuals();
	}

	SwimPhase = FMath::FRand() * PI * 2.f;
	PreferredDepth = -40.f - FMath::FRand() * 90.f;
	PickWanderTarget();

	if (ABFLGameMode* GM = UBFLStatics::GetBFLGameMode(this))
	{
		GM->RegisterFish(this);
	}
}

void ABFLFishActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ABFLGameMode* GM = UBFLStatics::GetBFLGameMode(this))
	{
		GM->UnregisterFish(this);
	}
	Super::EndPlay(EndPlayReason);
}

void ABFLFishActor::InitializeSpecies(const FBFLFishSpeciesDef& InDef)
{
	SpeciesDef = InDef;
	WeightLbs = FMath::FRandRange(SpeciesDef.MinWeightLbs, SpeciesDef.MaxWeightLbs);
	ApplyVisuals();
}

void ABFLFishActor::ApplyVisuals()
{
	const float Size = FMath::GetMappedRangeValueClamped(
		FVector2D(SpeciesDef.MinWeightLbs, SpeciesDef.MaxWeightLbs),
		FVector2D(0.85f, 1.25f),
		WeightLbs);

	SetActorScale3D(FVector(Size));
	if (BodyMesh)
	{
		BodyMesh->SetRelativeScale3D(BFLFishLook::BodyMeshScale(bHasAuthoredBodyMesh, SpeciesDef.MeshScale));
		if (bHasAuthoredBodyMesh)
		{
			const int32 NumSlots = BodyMesh->GetNumMaterials();
			for (int32 Slot = 0; Slot < NumSlots; ++Slot)
			{
				if (!BodyMesh->GetMaterial(Slot))
				{
					continue;
				}
				if (UMaterialInstanceDynamic* Mid = BodyMesh->CreateAndSetMaterialInstanceDynamic(Slot))
				{
					// Multiply/tint params only. Do not write BaseColor — that flattens stylized-PBR paint.
					Mid->SetVectorParameterValue(TEXT("Tint"), SpeciesDef.Color);
					Mid->SetVectorParameterValue(TEXT("Color"), SpeciesDef.Color);
					Mid->SetVectorParameterValue(TEXT("TintColor"), SpeciesDef.Color);
				}
			}
		}
		else if (UMaterialInstanceDynamic* BodyMat = UBFLStatics::MakeTintedMeshMaterial(this, SpeciesDef.Color))
		{
			BodyMesh->SetMaterial(0, BodyMat);
		}
	}
	if (TailMesh && TailMesh->GetStaticMesh() && BFLFishLook::ShouldShowTail(bHasAuthoredBodyMesh))
	{
		if (UMaterialInstanceDynamic* TailMat = UBFLStatics::MakeTintedMeshMaterial(this, SpeciesDef.BellyColor))
		{
			TailMesh->SetMaterial(0, TailMat);
		}
	}
}

void ABFLFishActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	StateTime += DeltaSeconds;
	InterestCooldown = FMath::Max(0.f, InterestCooldown - DeltaSeconds);
	SwimPhase += DeltaSeconds * 8.f;

	if (TailMesh && TailMesh->GetStaticMesh())
	{
		const float Wag = FMath::Sin(SwimPhase) * 18.f;
		TailMesh->SetRelativeRotation(FRotator(0.f, 180.f + Wag, 0.f));
	}

	switch (AIState)
	{
	case EFishAIState::Wander:
	case EFishAIState::Interested:
		TickWander(DeltaSeconds);
		break;
	case EFishAIState::Approaching:
		TickApproach(DeltaSeconds);
		break;
	case EFishAIState::DecidingBite:
		TickDecideBite(DeltaSeconds);
		break;
	case EFishAIState::Fighting:
		TickFight(DeltaSeconds);
		break;
	case EFishAIState::Flee:
		TickFlee(DeltaSeconds);
		break;
	}
}

void ABFLFishActor::SetAIState(EFishAIState NewState)
{
	AIState = NewState;
	StateTime = 0.f;
}

void ABFLFishActor::TickWander(float DeltaSeconds)
{
	MoveToward(WanderTarget, SpeciesDef.SwimSpeed, DeltaSeconds);

	if (FVector::DistSquared2D(GetActorLocation(), WanderTarget) < FMath::Square(ArriveThreshold)
		|| StateTime > 8.f)
	{
		PickWanderTarget();
		StateTime = 0.f;
	}

	if (InterestCooldown > 0.f)
	{
		return;
	}

	if (ABFLBaitActor* Bait = FindBait())
	{
		const float Dist = FVector::Dist(GetActorLocation(), Bait->GetActorLocation());
		if (Dist <= SpeciesDef.DetectRadius)
		{
			SetAIState(EFishAIState::Approaching);
		}
	}
}

void ABFLFishActor::TickApproach(float DeltaSeconds)
{
	ABFLBaitActor* Bait = FindBait();
	if (!Bait)
	{
		SetAIState(EFishAIState::Wander);
		PickWanderTarget();
		return;
	}

	const FVector BaitLoc = Bait->GetActorLocation();
	MoveToward(BaitLoc + FVector(0.f, 0.f, -20.f), SpeciesDef.SwimSpeed * 1.25f, DeltaSeconds);

	const float Dist = FVector::Dist(GetActorLocation(), BaitLoc);
	if (Dist <= SpeciesDef.BiteRadius)
	{
		BiteDelay = FMath::FRandRange(0.45f, 1.8f);
		SetAIState(EFishAIState::DecidingBite);
		return;
	}

	if (Dist > SpeciesDef.DetectRadius * 1.35f || StateTime > 10.f)
	{
		InterestCooldown = 2.5f;
		SetAIState(EFishAIState::Wander);
		PickWanderTarget();
	}
}

void ABFLFishActor::TickDecideBite(float DeltaSeconds)
{
	ABFLBaitActor* Bait = FindBait();
	if (!Bait)
	{
		SetAIState(EFishAIState::Wander);
		PickWanderTarget();
		return;
	}

	// Hold station under the bobber while "inspecting" the bait.
	const FVector Hover = Bait->GetActorLocation() + FVector(0.f, 0.f, -28.f);
	MoveToward(Hover, SpeciesDef.SwimSpeed * 0.45f, DeltaSeconds);

	if (StateTime >= BiteDelay)
	{
		TryCommitBite(Bait);
	}
}

void ABFLFishActor::TryCommitBite(ABFLBaitActor* Bait)
{
	if (!Bait || !Bait->IsAvailable())
	{
		InterestCooldown = 2.f;
		SetAIState(EFishAIState::Wander);
		PickWanderTarget();
		return;
	}

	const bool bBite = FMath::FRand() <= SpeciesDef.BiteChance;
	if (!bBite)
	{
		InterestCooldown = 4.f;
		SetAIState(EFishAIState::Wander);
		PickWanderTarget();
		return;
	}

	AActor* BaitOwner = Bait->GetOwner();
	UBFLFishingComponent* Fishing = BaitOwner
		? BaitOwner->FindComponentByClass<UBFLFishingComponent>()
		: nullptr;
	if (!Fishing)
	{
		SetAIState(EFishAIState::Wander);
		return;
	}

	FightCenter = Bait->GetActorLocation();
	FightAngle = FMath::FRand() * 2.f * PI;
	SetAIState(EFishAIState::Fighting);
	Fishing->NotifyFishBite(this);
}

void ABFLFishActor::TickFight(float DeltaSeconds)
{
	ABFLBaitActor* Bait = FindBait();
	if (!Bait)
	{
		Flee(4.f);
		return;
	}

	// Circle and tug so the bobber skitters across the surface.
	FightAngle += DeltaSeconds * (0.9f + SpeciesDef.FightIntensity * 0.8f);
	const float Radius = 90.f + SpeciesDef.FightIntensity * 80.f;
	const FVector Pull(
		FightCenter.X + FMath::Cos(FightAngle) * Radius,
		FightCenter.Y + FMath::Sin(FightAngle) * Radius,
		UBFLStatics::GetWaterHeight(this) - 25.f);

	MoveToward(Pull, SpeciesDef.SwimSpeed * 1.4f, DeltaSeconds);

	FVector Bobber = GetActorLocation();
	Bobber.Z = UBFLStatics::GetWaterHeight(this) + 4.f;
	Bait->SetActorLocation(Bobber);
}

void ABFLFishActor::TickFlee(float DeltaSeconds)
{
	FleeTimeLeft -= DeltaSeconds;
	MoveToward(WanderTarget, SpeciesDef.SwimSpeed * 1.8f, DeltaSeconds);

	if (FleeTimeLeft <= 0.f || FVector::DistSquared2D(GetActorLocation(), WanderTarget) < FMath::Square(ArriveThreshold))
	{
		InterestCooldown = 3.f;
		SetAIState(EFishAIState::Wander);
		PickWanderTarget();
	}
}

void ABFLFishActor::OnLineReleased(bool bCaught)
{
	if (bCaught)
	{
		Destroy();
		return;
	}
	Flee(5.f);
}

void ABFLFishActor::Flee(float Duration)
{
	if (AIState == EFishAIState::Fighting)
	{
		// The fishing component owns fight teardown; we just run.
	}

	FleeTimeLeft = Duration;
	const FVector Center = UBFLStatics::GetLakeCenter(this);
	const FVector Away = (GetActorLocation() - Center).GetSafeNormal2D();
	const float Radius = UBFLStatics::GetLakeRadius(this) * 0.8f;
	WanderTarget = Center + Away * Radius;
	WanderTarget.Z = UBFLStatics::GetWaterHeight(this) + PreferredDepth;
	SetAIState(EFishAIState::Flee);
}

void ABFLFishActor::PickWanderTarget()
{
	const FVector Center = UBFLStatics::GetLakeCenter(this);
	const float Radius = UBFLStatics::GetLakeRadius(this) * 0.85f;
	const float WaterZ = UBFLStatics::GetWaterHeight(this);
	PreferredDepth = -40.f - FMath::FRand() * 90.f;
	WanderTarget = UBFLStatics::RandomPointInLake(Center, Radius, WaterZ, PreferredDepth, 0.1f);
}

void ABFLFishActor::MoveToward(const FVector& Target, float Speed, float DeltaSeconds)
{
	const FVector Center = UBFLStatics::GetLakeCenter(this);
	const float Radius = UBFLStatics::GetLakeRadius(this) * 0.9f;
	const float WaterZ = UBFLStatics::GetWaterHeight(this);

	FVector Desired = Target;
	Desired = UBFLStatics::ClampToLake(Desired, Center, Radius, Desired.Z, 0.f);

	FVector To = Desired - GetActorLocation();
	FVector Flat(To.X, To.Y, 0.f);
	if (Flat.SizeSquared() > 1.f)
	{
		const FRotator DesiredRot = Flat.Rotation();
		const FRotator NewRot = FMath::RInterpConstantTo(GetActorRotation(), DesiredRot, DeltaSeconds, TurnSpeed);
		SetActorRotation(NewRot);
	}

	const FVector Step = To.GetSafeNormal() * Speed * DeltaSeconds;
	FVector Next = GetActorLocation() + Step;
	Next = UBFLStatics::ClampToLake(Next, Center, Radius, WaterZ, Next.Z - WaterZ);

	// Gentle vertical swim-bob so they don't look like sliding volumes.
	Next.Z += FMath::Sin(SwimPhase * 0.35f) * 2.f;
	SetActorLocation(Next);
}

ABFLBaitActor* ABFLFishActor::FindBait() const
{
	return UBFLStatics::GetActiveBait(this);
}

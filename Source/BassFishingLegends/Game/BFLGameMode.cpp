#include "Game/BFLGameMode.h"

#include "BassFishingLegends.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Fish/BFLFishActor.h"
#include "Fish/BFLFishCatalog.h"
#include "Fishing/BFLBaitActor.h"
#include "Game/BFLGameSettings.h"
#include "Game/BFLPlayerController.h"
#include "Game/BFLStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BFLBoatPawn.h"
#include "UI/BFLHUD.h"
#include "World/BFLDayNightActor.h"

ABFLGameMode::ABFLGameMode()
{
	DefaultPawnClass = ABFLBoatPawn::StaticClass();
	PlayerControllerClass = ABFLPlayerController::StaticClass();
	HUDClass = ABFLHUD::StaticClass();

	ApplySettingsDefaults();
}

void ABFLGameMode::ApplySettingsDefaults()
{
	if (const UBFLGameSettings* Settings = UBFLGameSettings::Get())
	{
		WaterHeight = Settings->WaterHeight;
		LakeRadius = Settings->LakeRadius;
		FishCount = Settings->FishCount;
		bSpawnDayNight = Settings->bEnableDayNight;
	}
}

void ABFLGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	ApplySettingsDefaults();
}

void ABFLGameMode::StartPlay()
{
	if (bAutoBuildLake)
	{
		bool bWaterExists = false;
		if (bSkipBuildIfWaterExists && GetWorld())
		{
			for (TActorIterator<AActor> It(GetWorld()); It; ++It)
			{
				if (It->GetName().Contains(TEXT("WaterSurface")) || It->ActorHasTag(FName(TEXT("WaterSurface"))))
				{
					bWaterExists = true;
					LakeCenter = It->GetActorLocation();
					LakeCenter.Z = 0.f;
					WaterHeight = It->GetActorLocation().Z;
					break;
				}
			}
		}

		if (!bWaterExists)
		{
			BuildDefaultLake();
		}
	}

	EnsurePlayerStart();
	EnsureLighting();

	if (bSpawnDayNight)
	{
		bool bHasDayNight = false;
		for (TActorIterator<ABFLDayNightActor> It(GetWorld()); It; ++It)
		{
			bHasDayNight = true;
			break;
		}
		if (!bHasDayNight)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			GetWorld()->SpawnActor<ABFLDayNightActor>(ABFLDayNightActor::StaticClass(), FTransform::Identity, Params);
		}
	}

	Super::StartPlay();

	SpawnStartingFish();

	UE_LOG(LogBFL, Log, TEXT("Bass Fishing Legends ready. LakeRadius=%.0f Fish=%d"), LakeRadius, FishCount);
}

void ABFLGameMode::RegisterBait(ABFLBaitActor* Bait)
{
	if (ActiveBait.IsValid() && ActiveBait.Get() != Bait)
	{
		ActiveBait->Destroy();
	}
	ActiveBait = Bait;
}

void ABFLGameMode::UnregisterBait(ABFLBaitActor* Bait)
{
	if (ActiveBait.Get() == Bait)
	{
		ActiveBait = nullptr;
	}
}

void ABFLGameMode::RegisterFish(ABFLFishActor* Fish)
{
	LiveFish.AddUnique(Fish);
}

void ABFLGameMode::UnregisterFish(ABFLFishActor* Fish)
{
	LiveFish.RemoveAll([Fish](const TWeakObjectPtr<ABFLFishActor>& Ptr)
	{
		return !Ptr.IsValid() || Ptr.Get() == Fish;
	});
}

void ABFLGameMode::NotifyAllFishFlee(float Duration)
{
	for (const TWeakObjectPtr<ABFLFishActor>& Ptr : LiveFish)
	{
		if (ABFLFishActor* Fish = Ptr.Get())
		{
			Fish->Flee(Duration);
		}
	}
}

FBFLCatchRecord ABFLGameMode::RegisterCatch(EFishSpecies Species, float WeightLbs)
{
	const FBFLFishSpeciesDef Def = GetSpeciesDef(Species);
	FBFLCatchRecord Record;
	Record.Species = Species;
	Record.DisplayName = Def.DisplayName;
	Record.WeightLbs = WeightLbs;
	Record.ScoreAwarded = FMath::RoundToInt(WeightLbs * Def.ScorePerPound);
	Record.TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	Score += Record.ScoreAwarded;
	++FishCaught;
	CatchLog.Add(Record);

	OnFishCaught.Broadcast(Species, WeightLbs);

	const FText Message = FText::Format(
		NSLOCTEXT("BFL", "CaughtFmt", "Fish caught! {0}  {1} lb   +{2}"),
		Def.DisplayName,
		FText::AsNumber(FMath::RoundToFloat(WeightLbs * 10.f) / 10.f),
		FText::AsNumber(Record.ScoreAwarded));
	BroadcastNotification(Message, 3.0f);

	UE_LOG(LogBFL, Log, TEXT("Caught %s (%.1f lb) +%d  Total=%d"),
		*Def.DisplayName.ToString(), WeightLbs, Record.ScoreAwarded, Score);

	return Record;
}

FBFLFishSpeciesDef ABFLGameMode::GetSpeciesDef(EFishSpecies Species) const
{
	if (FishCatalog)
	{
		return FishCatalog->GetDefinition(Species);
	}
	return UBFLFishCatalog::MakeBuiltin(Species);
}

ABFLFishActor* ABFLGameMode::SpawnFish(EFishSpecies Species, FVector Location)
{
	if (!GetWorld())
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ABFLFishActor* Fish = GetWorld()->SpawnActor<ABFLFishActor>(ABFLFishActor::StaticClass(), Location, FRotator::ZeroRotator, Params);
	if (Fish)
	{
		Fish->InitializeSpecies(GetSpeciesDef(Species));
	}
	return Fish;
}

void ABFLGameMode::BroadcastNotification(const FText& Message, float Duration)
{
	OnNotification.Broadcast(Message);
	if (ABFLHUD* HUD = Cast<ABFLHUD>(UGameplayStatics::GetPlayerController(this, 0) ? UGameplayStatics::GetPlayerController(this, 0)->GetHUD() : nullptr))
	{
		HUD->ShowNotification(Message, Duration);
	}
}

void ABFLGameMode::BuildDefaultLake()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UStaticMesh* Plane = UBFLStatics::GetEngineMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	UStaticMesh* Cube = UBFLStatics::GetEngineMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (!Plane || !Cube)
	{
		UE_LOG(LogBFL, Warning, TEXT("Could not load engine basic shapes. Lake visual will be missing."));
		return;
	}

	// Water surface: engine plane is 100x100 units, scale to diameter.
	const float PlaneScale = (LakeRadius * 2.f) / 100.f;
	UStaticMeshComponent* Water = SpawnColoredMesh(
		Plane,
		FVector(LakeCenter.X, LakeCenter.Y, WaterHeight),
		FRotator::ZeroRotator,
		FVector(PlaneScale, PlaneScale, 1.f),
		FLinearColor(0.04f, 0.22f, 0.38f, 1.f),
		FName(TEXT("WaterSurface")),
		false);
	if (Water)
	{
		if (AActor* Owner = Water->GetOwner())
		{
			Owner->Tags.AddUnique(FName(TEXT("WaterSurface")));
		}
	}

	// Darker "depth" plate just below the surface.
	SpawnColoredMesh(
		Plane,
		FVector(LakeCenter.X, LakeCenter.Y, WaterHeight - 80.f),
		FRotator::ZeroRotator,
		FVector(PlaneScale * 0.98f, PlaneScale * 0.98f, 1.f),
		FLinearColor(0.01f, 0.05f, 0.10f, 1.f),
		FName(TEXT("WaterDepth")),
		false);

	// Soft shore ring made of boxes just outside the swim radius.
	const int32 ShorePieces = 28;
	const float ShoreRadius = LakeRadius + 180.f;
	for (int32 i = 0; i < ShorePieces; ++i)
	{
		const float Angle = (2.f * PI * i) / ShorePieces;
		const FVector Loc(
			LakeCenter.X + FMath::Cos(Angle) * ShoreRadius,
			LakeCenter.Y + FMath::Sin(Angle) * ShoreRadius,
			WaterHeight + 20.f);
		const FRotator Rot(0.f, FMath::RadiansToDegrees(Angle), 0.f);
		SpawnColoredMesh(
			Cube,
			Loc,
			Rot,
			FVector(3.6f, 8.5f, 0.7f),
			FLinearColor(0.28f, 0.24f, 0.14f),
			FName(*FString::Printf(TEXT("Shore_%d"), i)),
			true);
	}

	SpawnAtmosphere();
}

void ABFLGameMode::SpawnAtmosphere()
{
	UStaticMesh* Cylinder = UBFLStatics::GetEngineMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* Cone = UBFLStatics::GetEngineMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
	UStaticMesh* Sphere = UBFLStatics::GetEngineMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (!Cylinder || !Cone || !Sphere)
	{
		return;
	}

	FRandomStream Rng(1337);

	for (int32 i = 0; i < TreeCount; ++i)
	{
		const float Angle = Rng.FRand() * 2.f * PI;
		const float R = LakeRadius + 280.f + Rng.FRand() * 700.f;
		const FVector Base(
			LakeCenter.X + FMath::Cos(Angle) * R,
			LakeCenter.Y + FMath::Sin(Angle) * R,
			WaterHeight + 70.f);

		const float TrunkH = 1.4f + Rng.FRand() * 1.1f;
		SpawnColoredMesh(
			Cylinder,
			Base,
			FRotator::ZeroRotator,
			FVector(0.28f, 0.28f, TrunkH),
			FLinearColor(0.22f, 0.13f, 0.07f),
			FName(*FString::Printf(TEXT("Trunk_%d"), i)));

		SpawnColoredMesh(
			Cone,
			Base + FVector(0.f, 0.f, 80.f * TrunkH + 40.f),
			FRotator::ZeroRotator,
			FVector(1.6f + Rng.FRand() * 0.7f, 1.6f + Rng.FRand() * 0.7f, 2.0f + Rng.FRand()),
			FLinearColor(0.08f + Rng.FRand() * 0.08f, 0.28f + Rng.FRand() * 0.16f, 0.08f),
			FName(*FString::Printf(TEXT("Canopy_%d"), i)),
			false);
	}

	for (int32 i = 0; i < RockCount; ++i)
	{
		const float Angle = Rng.FRand() * 2.f * PI;
		const float R = LakeRadius - 80.f + Rng.FRand() * 260.f;
		const FVector Loc(
			LakeCenter.X + FMath::Cos(Angle) * R,
			LakeCenter.Y + FMath::Sin(Angle) * R,
			WaterHeight + 18.f + Rng.FRand() * 20.f);
		const FVector Scale(0.6f + Rng.FRand() * 1.4f, 0.6f + Rng.FRand() * 1.4f, 0.35f + Rng.FRand() * 0.7f);
		SpawnColoredMesh(
			Sphere,
			Loc,
			FRotator(Rng.FRand() * 40.f, Rng.FRand() * 360.f, Rng.FRand() * 40.f),
			Scale,
			FLinearColor(0.28f, 0.27f, 0.25f),
			FName(*FString::Printf(TEXT("Rock_%d"), i)));
	}

	// A couple of small islands so the lake isn't a perfect disc.
	UStaticMesh* Cube = UBFLStatics::GetEngineMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube)
	{
		const FVector IslandA = LakeCenter + FVector(LakeRadius * 0.35f, -LakeRadius * 0.2f, WaterHeight + 25.f);
		SpawnColoredMesh(Cube, IslandA, FRotator::ZeroRotator, FVector(6.f, 4.5f, 0.55f), FLinearColor(0.32f, 0.28f, 0.16f), FName(TEXT("IslandA")));
		SpawnColoredMesh(Cylinder, IslandA + FVector(40.f, 20.f, 80.f), FRotator::ZeroRotator, FVector(0.25f, 0.25f, 1.2f), FLinearColor(0.2f, 0.12f, 0.06f), FName(TEXT("IslandATrunk")));
		SpawnColoredMesh(Cone, IslandA + FVector(40.f, 20.f, 180.f), FRotator::ZeroRotator, FVector(1.5f, 1.5f, 1.8f), FLinearColor(0.1f, 0.32f, 0.1f), FName(TEXT("IslandATree")), false);
	}
}

void ABFLGameMode::SpawnStartingFish()
{
	// Don't double-spawn if a designer already placed fish.
	int32 Existing = 0;
	for (TActorIterator<ABFLFishActor> It(GetWorld()); It; ++It)
	{
		++Existing;
	}
	const int32 ToSpawn = FMath::Max(0, FishCount - Existing);
	if (ToSpawn <= 0)
	{
		return;
	}

	UBFLFishCatalog* TempCatalog = NewObject<UBFLFishCatalog>(this);
	if (FishCatalog)
	{
		TempCatalog->Species = FishCatalog->Species;
	}

	for (int32 i = 0; i < ToSpawn; ++i)
	{
		const EFishSpecies Species = TempCatalog->PickWeightedSpecies();
		const float Depth = -40.f - FMath::FRand() * 90.f;
		const FVector Loc = UBFLStatics::RandomPointInLake(LakeCenter, LakeRadius * 0.88f, WaterHeight, Depth, 0.12f);
		SpawnFish(Species, Loc);
	}
}

void ABFLGameMode::EnsurePlayerStart()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FVector Loc(LakeCenter.X, LakeCenter.Y, WaterHeight + 40.f);
	World->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), Loc, FRotator::ZeroRotator, Params);
}

void ABFLGameMode::EnsureLighting()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	bool bHasSun = false;
	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		bHasSun = true;
		break;
	}
	if (!bHasSun)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), FVector(0.f, 0.f, 400.f), FRotator(-40.f, 30.f, 0.f), Params);
		if (Sun)
		{
			if (UDirectionalLightComponent* Light = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
			{
				Light->SetIntensity(8.f);
				Light->SetLightColor(FLinearColor(1.f, 0.96f, 0.88f));
				Light->SetAtmosphereSunLight(true);
			}
			Sun->Tags.Add(FName(TEXT("BFL_Sun")));
		}
	}

	bool bHasSky = false;
	for (TActorIterator<ASkyLight> It(World); It; ++It)
	{
		bHasSky = true;
		break;
	}
	if (!bHasSky)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<ASkyLight>(ASkyLight::StaticClass(), FVector(0.f, 0.f, 300.f), FRotator::ZeroRotator, Params);
	}
}

UStaticMeshComponent* ABFLGameMode::SpawnColoredMesh(
	UStaticMesh* Mesh,
	const FVector& Location,
	const FRotator& Rotation,
	const FVector& Scale,
	const FLinearColor& Color,
	const FName& Name,
	bool bCollision)
{
	UWorld* World = GetWorld();
	if (!World || !Mesh)
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.Name = MakeUniqueObjectName(World, AStaticMeshActor::StaticClass(), Name);
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, Params);
	if (!Actor)
	{
		return nullptr;
	}

	Actor->SetActorScale3D(Scale);
	UStaticMeshComponent* Comp = Actor->GetStaticMeshComponent();
	Comp->SetStaticMesh(Mesh);
	Comp->SetMobility(EComponentMobility::Movable);
	if (UMaterialInstanceDynamic* MID = UBFLStatics::MakeTintedMeshMaterial(Actor, Color))
	{
		Comp->SetMaterial(0, MID);
	}
	if (!bCollision)
	{
		Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Comp->SetCollisionResponseToAllChannels(ECR_Block);
	}
	return Comp;
}

#include "Fishing/BFLFishingComponent.h"

#include "CableComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Fish/BFLFishActor.h"
#include "Fishing/BFLBaitActor.h"
#include "Fishing/BFLCastOrigin.h"
#include "Fishing/BFLCastProjectile.h"
#include "Game/BFLGameMode.h"
#include "Game/BFLGameSettings.h"
#include "Game/BFLStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UBFLFishingComponent::UBFLFishingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	ProjectileClass = ABFLCastProjectile::StaticClass();
	BaitClass = ABFLBaitActor::StaticClass();
}

FBFLFightRates UBFLFishingComponent::GetFightRates() const
{
	FBFLFightRates Rates;
	Rates.ReelTensionRate = ReelTensionRate;
	Rates.TensionDecayRate = TensionDecayRate;
	Rates.IdleTensionFloor = IdleTensionFloor;
	Rates.OverreelPenalty = OverreelPenalty;
	Rates.YellowTension = YellowTension;
	Rates.RedTension = RedTension;
	Rates.FishPullToTension = FishPullToTension;
	Rates.ReelLeakRate = ReelLeakRate;
	Rates.ReelGainScale = ReelGainScale;
	return Rates;
}

void UBFLFishingComponent::BeginPlay()
{
	Super::BeginPlay();
	SetState(EFishingState::Idle);
}

void UBFLFishingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	StateTime += DeltaTime;
	UpdateCable();

	switch (State)
	{
	case EFishingState::ChargingCast:
		TickCharging(DeltaTime);
		break;
	case EFishingState::Fighting:
		TickFighting(DeltaTime);
		break;
	case EFishingState::Caught:
	case EFishingState::Failed:
		TickResolved(DeltaTime);
		break;
	default:
		break;
	}
}

void UBFLFishingComponent::StartCastCharge()
{
	if (State == EFishingState::Fighting)
	{
		StartReeling();
		return;
	}

	if (State == EFishingState::Waiting || State == EFishingState::Casting)
	{
		return;
	}

	if (!CanCast())
	{
		return;
	}

	CastPower = 0.05f;
	SetState(EFishingState::ChargingCast);
	OnCastPowerChanged.Broadcast(CastPower);
}

void UBFLFishingComponent::ReleaseCast()
{
	if (State == EFishingState::Fighting)
	{
		StopReeling();
		return;
	}

	if (State != EFishingState::ChargingCast)
	{
		return;
	}

	SpawnProjectile();
}

void UBFLFishingComponent::Cancel()
{
	if (State == EFishingState::Idle || State == EFishingState::Caught || State == EFishingState::Failed)
	{
		return;
	}

	if (HookedFish)
	{
		HookedFish->OnLineReleased(false);
		HookedFish = nullptr;
	}

	ClearLineActors();
	bReeling = false;
	Tension = 0.f;
	ReelProgress = 0.f;
	CastPower = 0.f;
	SetState(EFishingState::Idle);
}

void UBFLFishingComponent::StartReeling()
{
	if (State == EFishingState::Fighting)
	{
		bReeling = true;
	}
}

void UBFLFishingComponent::StopReeling()
{
	bReeling = false;
}

void UBFLFishingComponent::NotifyCastHitWater(const FVector& HitLocation)
{
	Projectile = nullptr;

	const FVector Center = UBFLStatics::GetLakeCenter(this);
	const float Radius = UBFLStatics::GetLakeRadius(this);
	if (!UBFLStatics::IsOverWater(HitLocation, Center, Radius))
	{
		if (ABFLGameMode* GM = UBFLStatics::GetBFLGameMode(this))
		{
			GM->BroadcastNotification(NSLOCTEXT("BFL", "ShoreCast", "Cast landed on shore."), 1.8f);
		}
		SetState(EFishingState::Idle);
		CastPower = 0.f;
		return;
	}

	SpawnBait(HitLocation);
}

void UBFLFishingComponent::NotifyCastMissed()
{
	Projectile = nullptr;
	CastPower = 0.f;
	SetState(EFishingState::Idle);
}

void UBFLFishingComponent::NotifyFishBite(ABFLFishActor* Fish)
{
	if (!Fish || State != EFishingState::Waiting || !Bait)
	{
		return;
	}

	HookedFish = Fish;
	Bait->HookFish(Fish);
	Tension = GetFightRates().StartTension;
	ReelProgress = 0.f;
	bReeling = false;
	FightPulsePhase = FMath::FRand() * PI * 2.f;
	SetState(EFishingState::Fighting);

	if (ABFLGameMode* GM = UBFLStatics::GetBFLGameMode(this))
	{
		const FText SpeciesName = Fish->GetSpeciesDef().DisplayName;
		GM->BroadcastNotification(FText::Format(NSLOCTEXT("BFL", "FishOn", "FISH ON!  {0}"), SpeciesName), 2.0f);
	}
}

bool UBFLFishingComponent::PredictLanding(FVector& OutLocation) const
{
	const float Power = (State == EFishingState::ChargingCast) ? CastPower : 0.55f;
	return UBFLStatics::PredictWaterHit(
		GetCastStart(),
		GetCastVelocity(Power),
		GetGravityZ(),
		UBFLStatics::GetWaterHeight(this),
		5.f,
		OutLocation);
}

void UBFLFishingComponent::SetState(EFishingState NewState)
{
	if (State == NewState && NewState != EFishingState::Idle)
	{
		// Still refresh timers when we force Idle from Idle after a resolved catch.
	}

	State = NewState;
	StateTime = 0.f;
	OnStateChanged.Broadcast(State);
}

void UBFLFishingComponent::TickCharging(float DeltaTime)
{
	const float Rate = 1.f / FMath::Max(ChargeTime, 0.1f);
	CastPower = FMath::Clamp(CastPower + Rate * DeltaTime, 0.f, 1.f);
	OnCastPowerChanged.Broadcast(CastPower);
}

void UBFLFishingComponent::TickFighting(float DeltaTime)
{
	if (!HookedFish || !IsValid(HookedFish))
	{
		SnapLine();
		return;
	}

	const FBFLFishSpeciesDef& Def = HookedFish->GetSpeciesDef();
	const FBFLFightRates Rates = GetFightRates();

	FBFLFightState FightState;
	FightState.Tension = Tension;
	FightState.Reel = ReelProgress;
	FightState.PulsePhase = FightPulsePhase;

	const EBFLFightOutcome Outcome = BFLFightMath::Tick(
		FightState,
		bReeling,
		Def.FightIntensity,
		Def.StaminaSeconds,
		DeltaTime,
		Rates);

	Tension = FightState.Tension;
	ReelProgress = FightState.Reel;
	FightPulsePhase = FightState.PulsePhase;

	OnTensionChanged.Broadcast(Tension, ReelProgress);

	if (Outcome == EBFLFightOutcome::Snap)
	{
		SnapLine();
		return;
	}

	if (Outcome == EBFLFightOutcome::Land)
	{
		LandFish();
	}
}

void UBFLFishingComponent::TickResolved(float DeltaTime)
{
	const float Hold = (State == EFishingState::Caught) ? SuccessHoldTime : FailResetTime;
	if (StateTime >= Hold)
	{
		ClearLineActors();
		HookedFish = nullptr;
		bReeling = false;
		Tension = 0.f;
		ReelProgress = 0.f;
		CastPower = 0.f;
		SetState(EFishingState::Idle);
		OnTensionChanged.Broadcast(0.f, 0.f);
	}
}

void UBFLFishingComponent::SpawnProjectile()
{
	UWorld* World = GetWorld();
	if (!World || !ProjectileClass)
	{
		SetState(EFishingState::Idle);
		return;
	}

	const FVector Start = GetCastStart();
	const FVector Vel = GetCastVelocity(CastPower);

	FActorSpawnParameters Params;
	Params.Owner = GetOwner();
	Params.Instigator = Cast<APawn>(GetOwner());
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ABFLCastProjectile* Lure = World->SpawnActor<ABFLCastProjectile>(ProjectileClass, Start, Vel.Rotation(), Params);
	if (!Lure)
	{
		SetState(EFishingState::Idle);
		return;
	}

	Lure->Launch(Vel, UBFLStatics::GetWaterHeight(this), this);
	Projectile = Lure;
	SetState(EFishingState::Casting);
}

void UBFLFishingComponent::SpawnBait(const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World || !BaitClass)
	{
		SetState(EFishingState::Idle);
		return;
	}

	FVector SpawnLoc = Location;
	SpawnLoc.Z = UBFLStatics::GetWaterHeight(this) + 6.f;

	FActorSpawnParameters Params;
	Params.Owner = GetOwner();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	Bait = World->SpawnActor<ABFLBaitActor>(BaitClass, SpawnLoc, FRotator::ZeroRotator, Params);
	if (!Bait)
	{
		SetState(EFishingState::Idle);
		return;
	}

	SetState(EFishingState::Waiting);
}

void UBFLFishingComponent::ClearLineActors()
{
	if (Projectile)
	{
		Projectile->Destroy();
		Projectile = nullptr;
	}
	if (Bait)
	{
		Bait->Destroy();
		Bait = nullptr;
	}
	if (LineCable)
	{
		LineCable->SetVisibility(false);
		LineCable->SetAttachEndTo(nullptr, NAME_None);
	}
}

void UBFLFishingComponent::UpdateCable()
{
	if (!LineCable)
	{
		return;
	}

	AActor* EndActor = nullptr;
	if (Projectile)
	{
		EndActor = Projectile;
	}
	else if (Bait)
	{
		EndActor = Bait;
	}

	if (!EndActor || State == EFishingState::Idle || State == EFishingState::ChargingCast)
	{
		LineCable->SetVisibility(false);
		return;
	}

	LineCable->SetVisibility(true);
	LineCable->SetAttachEndTo(EndActor, NAME_None, NAME_None);
	LineCable->EndLocation = FVector::ZeroVector;
	LineCable->bAttachEnd = true;
}

void UBFLFishingComponent::LandFish()
{
	if (!HookedFish)
	{
		SetState(EFishingState::Idle);
		return;
	}

	const FBFLFishSpeciesDef Def = HookedFish->GetSpeciesDef();
	const float Weight = HookedFish->GetWeightLbs();

	HookedFish->OnLineReleased(true);
	if (ABFLGameMode* GM = UBFLStatics::GetBFLGameMode(this))
	{
		GM->RegisterCatch(Def.Species, Weight);
	}

	OnFishCaught.Broadcast(Def.Species, Weight);
	HookedFish = nullptr;
	bReeling = false;
	SetState(EFishingState::Caught);
}

void UBFLFishingComponent::SnapLine()
{
	if (HookedFish)
	{
		HookedFish->OnLineReleased(false);
		HookedFish = nullptr;
	}

	if (ABFLGameMode* GM = UBFLStatics::GetBFLGameMode(this))
	{
		GM->NotifyAllFishFlee(5.f);
		GM->BroadcastNotification(NSLOCTEXT("BFL", "Snapped", "Line snapped! The fish fled."), 2.4f);
	}

	OnLineSnapped.Broadcast();
	bReeling = false;
	Tension = 1.f;
	SetState(EFishingState::Failed);
}

FVector UBFLFishingComponent::GetCastStart() const
{
	FVector TipLoc = FVector::ZeroVector;
	const FVector* Tip = nullptr;
	if (RodTip)
	{
		TipLoc = RodTip->GetComponentLocation();
		Tip = &TipLoc;
	}

	FVector OverrideLoc = FVector::ZeroVector;
	const FVector* Override = nullptr;
	if (CastOriginOverride)
	{
		OverrideLoc = CastOriginOverride->GetComponentLocation();
		Override = &OverrideLoc;
	}

	FVector Fallback = FVector::ZeroVector;
	if (const AActor* OwnerActor = GetOwner())
	{
		Fallback = OwnerActor->GetActorLocation() + OwnerActor->GetActorForwardVector() * 80.f + FVector(0.f, 0.f, 80.f);
	}

	return BFLCastOrigin::Resolve(Tip, Override, Fallback);
}

FVector UBFLFishingComponent::GetCastDirection() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	FVector Dir = Pawn ? Pawn->GetActorForwardVector() : FVector::ForwardVector;

	if (const APawn* P = Cast<APawn>(GetOwner()))
	{
		if (const APlayerController* PC = Cast<APlayerController>(P->GetController()))
		{
			FVector CamLoc;
			FRotator CamRot;
			PC->GetPlayerViewPoint(CamLoc, CamRot);
			Dir = CamRot.Vector();
		}
	}

	Dir.Z = 0.f;
	if (!Dir.Normalize())
	{
		Dir = FVector::ForwardVector;
	}

	const float Loft = UBFLGameSettings::Get()->CastLoft;
	Dir = (Dir + FVector(0.f, 0.f, Loft)).GetSafeNormal();
	return Dir;
}

FVector UBFLFishingComponent::GetCastVelocity(float Power) const
{
	const UBFLGameSettings* Settings = UBFLGameSettings::Get();
	const float Speed = FMath::Lerp(Settings->MinCastSpeed, Settings->MaxCastSpeed, FMath::Clamp(Power, 0.f, 1.f));
	return GetCastDirection() * Speed;
}

float UBFLFishingComponent::GetGravityZ() const
{
	if (ProjectileClass)
	{
		if (const ABFLCastProjectile* CDO = ProjectileClass->GetDefaultObject<ABFLCastProjectile>())
		{
			return CDO->GravityZ;
		}
	}
	return -2200.f;
}

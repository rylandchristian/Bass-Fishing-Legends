#pragma once

#include "CoreMinimal.h"
#include "BFLTypes.h"
#include "Components/ActorComponent.h"
#include "Fishing/BFLFightMath.h"
#include "BFLFishingComponent.generated.h"

class ABFLBaitActor;
class ABFLCastProjectile;
class ABFLFishActor;
class UCableComponent;
class USceneComponent;

/**
 * Owns fishing: charge-cast, wait, fight, then land or snap.
 * Attach to the boat (or any pawn). Designed to be subclassed / called from Blueprints.
 */
UCLASS(ClassGroup = (BFL), Blueprintable, meta = (BlueprintSpawnableComponent))
class BASSFISHINGLEGENDS_API UBFLFishingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBFLFishingComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure, Category = "BFL|Fishing")
	EFishingState GetState() const { return State; }

	UFUNCTION(BlueprintPure, Category = "BFL|Fishing")
	float GetCastPower() const { return CastPower; }

	UFUNCTION(BlueprintPure, Category = "BFL|Fishing")
	float GetTension() const { return Tension; }

	UFUNCTION(BlueprintPure, Category = "BFL|Fishing")
	float GetReelProgress() const { return ReelProgress; }

	UFUNCTION(BlueprintPure, Category = "BFL|Fishing")
	bool IsReeling() const { return bReeling; }

	UFUNCTION(BlueprintPure, Category = "BFL|Fishing")
	ABFLBaitActor* GetBait() const { return Bait; }

	UFUNCTION(BlueprintPure, Category = "BFL|Fishing")
	ABFLFishActor* GetHookedFish() const { return HookedFish; }

	UFUNCTION(BlueprintPure, Category = "BFL|Fishing")
	bool CanCast() const { return State == EFishingState::Idle; }

	/** Hold to charge. */
	UFUNCTION(BlueprintCallable, Category = "BFL|Fishing")
	void StartCastCharge();

	/** Release to throw. */
	UFUNCTION(BlueprintCallable, Category = "BFL|Fishing")
	void ReleaseCast();

	UFUNCTION(BlueprintCallable, Category = "BFL|Fishing")
	void Cancel();

	UFUNCTION(BlueprintCallable, Category = "BFL|Fishing")
	void StartReeling();

	UFUNCTION(BlueprintCallable, Category = "BFL|Fishing")
	void StopReeling();

	/** Called by the lure when it crosses the water plane. */
	UFUNCTION(BlueprintCallable, Category = "BFL|Fishing")
	void NotifyCastHitWater(const FVector& HitLocation);

	UFUNCTION(BlueprintCallable, Category = "BFL|Fishing")
	void NotifyCastMissed();

	/** Called by a fish when it commits to a bite. */
	UFUNCTION(BlueprintCallable, Category = "BFL|Fishing")
	void NotifyFishBite(ABFLFishActor* Fish);

	UFUNCTION(BlueprintPure, Category = "BFL|Fishing")
	bool PredictLanding(FVector& OutLocation) const;

	void SetRodTip(USceneComponent* InRodTip) { RodTip = InRodTip; }
	void SetLineCable(UCableComponent* InCable) { LineCable = InCable; }
	void SetCastOrigin(USceneComponent* InOrigin) { CastOriginOverride = InOrigin; }

	UPROPERTY(BlueprintAssignable, Category = "BFL|Fishing")
	FOnBFLFishingStateChanged OnStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "BFL|Fishing")
	FOnBFLCastPowerChanged OnCastPowerChanged;

	UPROPERTY(BlueprintAssignable, Category = "BFL|Fishing")
	FOnBFLTensionChanged OnTensionChanged;

	UPROPERTY(BlueprintAssignable, Category = "BFL|Fishing")
	FOnBFLFishCaught OnFishCaught;

	UPROPERTY(BlueprintAssignable, Category = "BFL|Fishing")
	FOnBFLLineSnapped OnLineSnapped;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Cast")
	float ChargeTime = 1.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Cast")
	TSubclassOf<ABFLCastProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Cast")
	TSubclassOf<ABFLBaitActor> BaitClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fight")
	float ReelTensionRate = 0.28f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fight")
	float TensionDecayRate = 0.30f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fight")
	float IdleTensionFloor = 0.08f;

	/** Extra tension when the player keeps reeling in the red. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fight")
	float OverreelPenalty = 0.45f;

	/** Yellow band. Reel through this. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fight", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float YellowTension = 0.55f;

	/** Red band. Ease off. Overreel starts here. Reel does not advance here. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fight", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RedTension = 0.78f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fight")
	float FishPullToTension = 0.16f;

	/** Reel lost per second while easing off. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fight")
	float ReelLeakRate = 0.015f;

	/** Scales reel gained while reeling in green/yellow. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fight")
	float ReelGainScale = 2.4f;

	UFUNCTION(BlueprintPure, Category = "BFL|Fight")
	float GetYellowTension() const { return YellowTension; }

	UFUNCTION(BlueprintPure, Category = "BFL|Fight")
	float GetRedTension() const { return RedTension; }

	FBFLFightRates GetFightRates() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fight")
	float SuccessHoldTime = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fight")
	float FailResetTime = 1.2f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Fishing")
	EFishingState State = EFishingState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Fishing")
	float CastPower = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Fishing")
	float Tension = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Fishing")
	float ReelProgress = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Fishing")
	bool bReeling = false;

	UPROPERTY()
	TObjectPtr<ABFLBaitActor> Bait;

	UPROPERTY()
	TObjectPtr<ABFLCastProjectile> Projectile;

	UPROPERTY()
	TObjectPtr<ABFLFishActor> HookedFish;

	UPROPERTY()
	TObjectPtr<USceneComponent> RodTip;

	UPROPERTY()
	TObjectPtr<USceneComponent> CastOriginOverride;

	UPROPERTY()
	TObjectPtr<UCableComponent> LineCable;

	float StateTime = 0.f;
	float FightPulsePhase = 0.f;

	void SetState(EFishingState NewState);
	void TickCharging(float DeltaTime);
	void TickFighting(float DeltaTime);
	void TickResolved(float DeltaTime);
	void SpawnProjectile();
	void SpawnBait(const FVector& Location);
	void ClearLineActors();
	void UpdateCable();
	void LandFish();
	void SnapLine();
	FVector GetCastStart() const;
	FVector GetCastDirection() const;
	FVector GetCastVelocity(float Power) const;
	float GetGravityZ() const;
};

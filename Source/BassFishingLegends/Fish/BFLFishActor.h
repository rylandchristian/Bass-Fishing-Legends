#pragma once

#include "CoreMinimal.h"
#include "BFLTypes.h"
#include "GameFramework/Actor.h"
#include "BFLFishActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class ABFLBaitActor;

/**
 * One bass (or other species) with a tiny tick-based brain:
 * wander, notice bait, approach, roll a bite, fight, or flee.
 * No Behavior Tree required — swap this for an AIController later if you want.
 */
UCLASS(Blueprintable)
class BASSFISHINGLEGENDS_API ABFLFishActor : public AActor
{
	GENERATED_BODY()

public:
	ABFLFishActor();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "BFL|Fish")
	void InitializeSpecies(const FBFLFishSpeciesDef& InDef);

	UFUNCTION(BlueprintPure, Category = "BFL|Fish")
	const FBFLFishSpeciesDef& GetSpeciesDef() const { return SpeciesDef; }

	UFUNCTION(BlueprintPure, Category = "BFL|Fish")
	float GetWeightLbs() const { return WeightLbs; }

	UFUNCTION(BlueprintPure, Category = "BFL|Fish")
	EFishAIState GetAIState() const { return AIState; }

	/** Called by the fishing component when the fight ends. */
	UFUNCTION(BlueprintCallable, Category = "BFL|Fish")
	void OnLineReleased(bool bCaught);

	UFUNCTION(BlueprintCallable, Category = "BFL|Fish")
	void Flee(float Duration = 5.f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Fish")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Fish")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Fish")
	TObjectPtr<UStaticMeshComponent> TailMesh;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BFL|Fish")
	FBFLFishSpeciesDef SpeciesDef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Fish")
	EFishAIState AIState = EFishAIState::Wander;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL|Fish")
	float WeightLbs = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fish")
	float TurnSpeed = 220.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fish")
	float ArriveThreshold = 80.f;

	void SetAIState(EFishAIState NewState);
	void TickWander(float DeltaSeconds);
	void TickApproach(float DeltaSeconds);
	void TickDecideBite(float DeltaSeconds);
	void TickFight(float DeltaSeconds);
	void TickFlee(float DeltaSeconds);

	void PickWanderTarget();
	void MoveToward(const FVector& Target, float Speed, float DeltaSeconds);
	ABFLBaitActor* FindBait() const;
	void TryCommitBite(ABFLBaitActor* Bait);
	void ApplyVisuals();
	void ApplyPlaceholderMeshes();

	FVector WanderTarget = FVector::ZeroVector;
	FVector FightCenter = FVector::ZeroVector;
	float StateTime = 0.f;
	float BiteDelay = 1.f;
	float FleeTimeLeft = 0.f;
	float InterestCooldown = 0.f;
	float SwimPhase = 0.f;
	float FightAngle = 0.f;
	float PreferredDepth = -70.f;
	bool bHasAuthoredBodyMesh = false;
};

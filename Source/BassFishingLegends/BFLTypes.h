#pragma once

#include "CoreMinimal.h"
#include "BFLTypes.generated.h"

/** High-level fishing loop on the player. */
UENUM(BlueprintType)
enum class EFishingState : uint8
{
	Idle			UMETA(DisplayName = "Idle"),
	ChargingCast	UMETA(DisplayName = "Charging Cast"),
	Casting			UMETA(DisplayName = "Line In Air"),
	Waiting			UMETA(DisplayName = "Waiting For Bite"),
	Fighting		UMETA(DisplayName = "Fish On"),
	Caught			UMETA(DisplayName = "Fish Caught"),
	Failed			UMETA(DisplayName = "Failed")
};

/** Simple fish brain. */
UENUM(BlueprintType)
enum class EFishAIState : uint8
{
	Wander			UMETA(DisplayName = "Wander"),
	Interested		UMETA(DisplayName = "Interested"),
	Approaching		UMETA(DisplayName = "Approaching"),
	DecidingBite	UMETA(DisplayName = "Deciding Bite"),
	Fighting		UMETA(DisplayName = "Fighting"),
	Flee			UMETA(DisplayName = "Flee")
};

/** Built-in species. Add more here, then fill a definition in UBFLFishCatalog. */
UENUM(BlueprintType)
enum class EFishSpecies : uint8
{
	LargemouthBass	UMETA(DisplayName = "Largemouth Bass"),
	SmallmouthBass	UMETA(DisplayName = "Smallmouth Bass"),
	Crappie			UMETA(DisplayName = "Crappie"),
	TrophyBass		UMETA(DisplayName = "Trophy Bass")
};

USTRUCT(BlueprintType)
struct BASSFISHINGLEGENDS_API FBFLFishSpeciesDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fish")
	EFishSpecies Species = EFishSpecies::LargemouthBass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fish")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fish")
	FLinearColor Color = FLinearColor(0.15f, 0.45f, 0.18f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fish")
	FLinearColor BellyColor = FLinearColor(0.75f, 0.72f, 0.45f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fish", meta = (ClampMin = "0.1"))
	float MinWeightLbs = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fish", meta = (ClampMin = "0.1"))
	float MaxWeightLbs = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fish", meta = (ClampMin = "10.0"))
	float SwimSpeed = 180.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fish", meta = (ClampMin = "50.0"))
	float DetectRadius = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fish", meta = (ClampMin = "20.0"))
	float BiteRadius = 70.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fish", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BiteChance = 0.55f;

	/** Peak extra tension this species adds while fighting (0-1 per second scale). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fish", meta = (ClampMin = "0.05"))
	float FightIntensity = 0.55f;

	/** Seconds of perfect reeling required to land the fish. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fish", meta = (ClampMin = "1.0"))
	float StaminaSeconds = 6.0f;

	/** Relative spawn frequency. Trophy should stay rare. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fish", meta = (ClampMin = "0.0"))
	float SpawnWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fish")
	FVector MeshScale = FVector(1.f, 1.8f, 0.7f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Fish")
	float ScorePerPound = 10.f;
};

USTRUCT(BlueprintType)
struct BASSFISHINGLEGENDS_API FBFLCatchRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Catch")
	EFishSpecies Species = EFishSpecies::LargemouthBass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Catch")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Catch")
	float WeightLbs = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Catch")
	int32 ScoreAwarded = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BFL|Catch")
	float TimeSeconds = 0.f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBFLFishingStateChanged, EFishingState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBFLFishCaught, EFishSpecies, Species, float, WeightLbs);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBFLLineSnapped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBFLCastPowerChanged, float, Power);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBFLTensionChanged, float, Tension, float, ReelProgress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBFLNotification, const FText&, Message);

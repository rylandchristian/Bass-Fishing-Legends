#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "BFLGameSettings.generated.h"

/**
 * Project-wide knobs. Edit in Edit > Project Settings > Game > Bass Fishing Legends
 * or in Config/DefaultGame.ini under [/Script/BassFishingLegends.BFLGameSettings].
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Bass Fishing Legends"))
class BASSFISHINGLEGENDS_API UBFLGameSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UBFLGameSettings();

	virtual FName GetCategoryName() const override { return FName(TEXT("Game")); }
	virtual FName GetSectionName() const override { return FName(TEXT("Bass Fishing Legends")); }

	static const UBFLGameSettings* Get();

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lake")
	float WaterHeight = 0.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lake", meta = (ClampMin = "500.0"))
	float LakeRadius = 4500.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Lake", meta = (ClampMin = "1"))
	int32 FishCount = 12;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Casting", meta = (ClampMin = "100.0"))
	float MinCastSpeed = 900.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Casting", meta = (ClampMin = "200.0"))
	float MaxCastSpeed = 2800.f;

	/** Extra upward fraction added to the cast direction. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Casting", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CastLoft = 0.35f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Boat", meta = (ClampMin = "50.0"))
	float BoatMaxSpeed = 650.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "World")
	bool bEnableDayNight = true;

	/** Real-world minutes for a full 24-hour cycle. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "World", meta = (ClampMin = "1.0"))
	float DayLengthMinutes = 12.f;
};

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimationAsset.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
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

	/** Optional authored jon-boat hull. Leave empty to keep the placeholder box. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Look")
	TSoftObjectPtr<UStaticMesh> BoatHullMesh;

	/** Optional authored Rod. Leave empty for the two-piece editor mesh. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Look")
	TSoftObjectPtr<UStaticMesh> RodMesh;

	/** Optional UE5-skeleton Angler. Leave empty to hide the Angler. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Look")
	TSoftObjectPtr<USkeletalMesh> AnglerMesh;

	/** Optional posed still. Played as a looping single-node clip. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Look")
	TSoftObjectPtr<UAnimationAsset> AnglerPose;

	/** Optional largemouth body. Leave empty to keep the sphere-and-cone fish. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Look")
	TSoftObjectPtr<UStaticMesh> FishBodyMesh;
};

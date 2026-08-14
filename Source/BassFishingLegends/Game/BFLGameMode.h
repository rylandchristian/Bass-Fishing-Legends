#pragma once

#include "CoreMinimal.h"
#include "BFLTypes.h"
#include "GameFramework/GameModeBase.h"
#include "BFLGameMode.generated.h"

class ABFLBaitActor;
class ABFLFishActor;
class ABFLBoatPawn;
class ABFLDayNightActor;
class UBFLFishCatalog;
class UStaticMeshComponent;

/**
 * Owns score, the active bait, the fish roster, and (optionally) builds a playable
 * lake at runtime so Play works on a blank map.
 */
UCLASS()
class BASSFISHINGLEGENDS_API ABFLGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABFLGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;

	UFUNCTION(BlueprintPure, Category = "BFL")
	float GetWaterHeight() const { return WaterHeight; }

	UFUNCTION(BlueprintPure, Category = "BFL")
	float GetLakeRadius() const { return LakeRadius; }

	UFUNCTION(BlueprintPure, Category = "BFL")
	FVector GetLakeCenter() const { return LakeCenter; }

	UFUNCTION(BlueprintPure, Category = "BFL")
	int32 GetScore() const { return Score; }

	UFUNCTION(BlueprintPure, Category = "BFL")
	int32 GetFishCaught() const { return FishCaught; }

	UFUNCTION(BlueprintPure, Category = "BFL")
	const TArray<FBFLCatchRecord>& GetCatchLog() const { return CatchLog; }

	UFUNCTION(BlueprintPure, Category = "BFL")
	ABFLBaitActor* GetActiveBait() const { return ActiveBait.Get(); }

	UFUNCTION(BlueprintCallable, Category = "BFL")
	void RegisterBait(ABFLBaitActor* Bait);

	UFUNCTION(BlueprintCallable, Category = "BFL")
	void UnregisterBait(ABFLBaitActor* Bait);

	UFUNCTION(BlueprintCallable, Category = "BFL")
	void RegisterFish(ABFLFishActor* Fish);

	UFUNCTION(BlueprintCallable, Category = "BFL")
	void UnregisterFish(ABFLFishActor* Fish);

	UFUNCTION(BlueprintCallable, Category = "BFL")
	void NotifyAllFishFlee(float Duration = 5.f);

	UFUNCTION(BlueprintCallable, Category = "BFL")
	FBFLCatchRecord RegisterCatch(EFishSpecies Species, float WeightLbs);

	UFUNCTION(BlueprintPure, Category = "BFL")
	FBFLFishSpeciesDef GetSpeciesDef(EFishSpecies Species) const;

	UFUNCTION(BlueprintCallable, Category = "BFL")
	ABFLFishActor* SpawnFish(EFishSpecies Species, FVector Location);

	UFUNCTION(BlueprintCallable, Category = "BFL")
	void BroadcastNotification(const FText& Message, float Duration = 2.5f);

	UPROPERTY(BlueprintAssignable, Category = "BFL")
	FOnBFLFishCaught OnFishCaught;

	UPROPERTY(BlueprintAssignable, Category = "BFL")
	FOnBFLNotification OnNotification;

protected:
	/** If true (default), spawn water, shore props, fish, and lighting on a blank map. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BFL|Lake")
	bool bAutoBuildLake = true;

	/** Skip auto-build if a WaterSurface tag or Water Body is already in the map. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BFL|Lake")
	bool bSkipBuildIfWaterExists = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BFL|Lake")
	float WaterHeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BFL|Lake")
	float LakeRadius = 4500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BFL|Lake")
	FVector LakeCenter = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BFL|Lake")
	int32 FishCount = 12;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BFL|Lake")
	int32 TreeCount = 22;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BFL|Lake")
	int32 RockCount = 16;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BFL|Fish")
	TObjectPtr<UBFLFishCatalog> FishCatalog;

	/** Optional Blueprint fish that already carries the largemouth mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BFL|Fish")
	TSubclassOf<ABFLFishActor> FishClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BFL|World")
	bool bSpawnDayNight = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL")
	int32 Score = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL")
	int32 FishCaught = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BFL")
	TArray<FBFLCatchRecord> CatchLog;

	UPROPERTY(VisibleAnywhere, Category = "BFL")
	TWeakObjectPtr<ABFLBaitActor> ActiveBait;

	UPROPERTY()
	TArray<TWeakObjectPtr<ABFLFishActor>> LiveFish;

	void ApplySettingsDefaults();
	void BuildDefaultLake();
	void SpawnAtmosphere();
	void SpawnStartingFish();
	void EnsurePlayerStart();
	void EnsureLighting();

	UStaticMeshComponent* SpawnColoredMesh(
		UStaticMesh* Mesh,
		const FVector& Location,
		const FRotator& Rotation,
		const FVector& Scale,
		const FLinearColor& Color,
		const FName& ActorName,
		bool bCollision = true);
};

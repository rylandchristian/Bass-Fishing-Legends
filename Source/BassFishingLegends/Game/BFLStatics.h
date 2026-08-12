#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "BFLStatics.generated.h"

class ABFLGameMode;
class ABFLBaitActor;

/** Shared helpers so Blueprints and C++ stay on one water plane / lake radius. */
UCLASS()
class BASSFISHINGLEGENDS_API UBFLStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "BFL", meta = (WorldContext = "WorldContextObject"))
	static ABFLGameMode* GetBFLGameMode(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "BFL", meta = (WorldContext = "WorldContextObject"))
	static float GetWaterHeight(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "BFL", meta = (WorldContext = "WorldContextObject"))
	static float GetLakeRadius(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "BFL", meta = (WorldContext = "WorldContextObject"))
	static FVector GetLakeCenter(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "BFL", meta = (WorldContext = "WorldContextObject"))
	static ABFLBaitActor* GetActiveBait(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "BFL")
	static FVector ClampToLake(const FVector& Location, const FVector& Center, float Radius, float WaterZ, float DepthOffset);

	UFUNCTION(BlueprintPure, Category = "BFL")
	static FVector RandomPointInLake(const FVector& Center, float Radius, float WaterZ, float DepthOffset, float MinRadiusFraction = 0.15f);

	UFUNCTION(BlueprintPure, Category = "BFL")
	static bool IsOverWater(const FVector& Location, const FVector& Center, float Radius);

	/** Predict where a ballistic cast hits the water plane. */
	UFUNCTION(BlueprintPure, Category = "BFL")
	static bool PredictWaterHit(FVector Start, FVector Velocity, float GravityZ, float WaterZ, float MaxTime, FVector& OutHit);

	static UStaticMesh* GetEngineMesh(const TCHAR* Path);
	static UMaterialInterface* GetBasicShapeMaterial();
	static UMaterialInstanceDynamic* MakeTintedMeshMaterial(UObject* Outer, const FLinearColor& Color, bool bUnlit = false);
};

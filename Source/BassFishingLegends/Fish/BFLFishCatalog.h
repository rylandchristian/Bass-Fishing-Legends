#pragma once

#include "CoreMinimal.h"
#include "BFLTypes.h"
#include "Engine/DataAsset.h"
#include "BFLFishCatalog.generated.h"

/** Built-in species table. Subclass in Blueprint to add your own roster. */
UCLASS(BlueprintType)
class BASSFISHINGLEGENDS_API UBFLFishCatalog : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UBFLFishCatalog();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BFL|Fish")
	TArray<FBFLFishSpeciesDef> Species;

	UFUNCTION(BlueprintPure, Category = "BFL|Fish")
	FBFLFishSpeciesDef GetDefinition(EFishSpecies InSpecies) const;

	UFUNCTION(BlueprintPure, Category = "BFL|Fish")
	EFishSpecies PickWeightedSpecies() const;

	/** Fallback used when no Data Asset is assigned on the GameMode. */
	static FBFLFishSpeciesDef MakeBuiltin(EFishSpecies InSpecies);
	static void FillBuiltins(TArray<FBFLFishSpeciesDef>& OutSpecies);
};

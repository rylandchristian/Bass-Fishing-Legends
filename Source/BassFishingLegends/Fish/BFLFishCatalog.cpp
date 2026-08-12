#include "Fish/BFLFishCatalog.h"

UBFLFishCatalog::UBFLFishCatalog()
{
	FillBuiltins(Species);
}

FBFLFishSpeciesDef UBFLFishCatalog::GetDefinition(EFishSpecies InSpecies) const
{
	for (const FBFLFishSpeciesDef& Def : Species)
	{
		if (Def.Species == InSpecies)
		{
			return Def;
		}
	}
	return MakeBuiltin(InSpecies);
}

EFishSpecies UBFLFishCatalog::PickWeightedSpecies() const
{
	float Total = 0.f;
	for (const FBFLFishSpeciesDef& Def : Species)
	{
		Total += FMath::Max(0.f, Def.SpawnWeight);
	}
	if (Total <= 0.f)
	{
		return EFishSpecies::LargemouthBass;
	}

	float Roll = FMath::FRand() * Total;
	for (const FBFLFishSpeciesDef& Def : Species)
	{
		Roll -= FMath::Max(0.f, Def.SpawnWeight);
		if (Roll <= 0.f)
		{
			return Def.Species;
		}
	}
	return Species.Last().Species;
}

FBFLFishSpeciesDef UBFLFishCatalog::MakeBuiltin(EFishSpecies InSpecies)
{
	FBFLFishSpeciesDef Def;
	Def.Species = InSpecies;

	switch (InSpecies)
	{
	case EFishSpecies::SmallmouthBass:
		Def.DisplayName = NSLOCTEXT("BFL", "Smallmouth", "Smallmouth Bass");
		Def.Color = FLinearColor(0.35f, 0.28f, 0.16f);
		Def.BellyColor = FLinearColor(0.82f, 0.78f, 0.55f);
		Def.MinWeightLbs = 1.0f;
		Def.MaxWeightLbs = 5.5f;
		Def.SwimSpeed = 220.f;
		Def.DetectRadius = 800.f;
		Def.BiteChance = 0.50f;
		Def.FightIntensity = 0.70f;
		Def.StaminaSeconds = 5.5f;
		Def.SpawnWeight = 0.8f;
		Def.MeshScale = FVector(0.85f, 1.6f, 0.62f);
		Def.ScorePerPound = 12.f;
		break;

	case EFishSpecies::Crappie:
		Def.DisplayName = NSLOCTEXT("BFL", "Crappie", "Crappie");
		Def.Color = FLinearColor(0.62f, 0.64f, 0.58f);
		Def.BellyColor = FLinearColor(0.9f, 0.9f, 0.85f);
		Def.MinWeightLbs = 0.4f;
		Def.MaxWeightLbs = 2.2f;
		Def.SwimSpeed = 140.f;
		Def.DetectRadius = 650.f;
		Def.BiteChance = 0.75f;
		Def.FightIntensity = 0.28f;
		Def.StaminaSeconds = 3.2f;
		Def.SpawnWeight = 0.9f;
		Def.MeshScale = FVector(0.55f, 1.1f, 0.5f);
		Def.ScorePerPound = 8.f;
		break;

	case EFishSpecies::TrophyBass:
		Def.DisplayName = NSLOCTEXT("BFL", "Trophy", "Trophy Largemouth");
		Def.Color = FLinearColor(0.08f, 0.28f, 0.10f);
		Def.BellyColor = FLinearColor(0.7f, 0.62f, 0.28f);
		Def.MinWeightLbs = 8.0f;
		Def.MaxWeightLbs = 14.5f;
		Def.SwimSpeed = 200.f;
		Def.DetectRadius = 1100.f;
		Def.BiteChance = 0.28f;
		Def.FightIntensity = 0.95f;
		Def.StaminaSeconds = 11.0f;
		Def.SpawnWeight = 0.15f;
		Def.MeshScale = FVector(1.45f, 2.6f, 1.0f);
		Def.ScorePerPound = 25.f;
		break;

	case EFishSpecies::LargemouthBass:
	default:
		Def.DisplayName = NSLOCTEXT("BFL", "Largemouth", "Largemouth Bass");
		Def.Color = FLinearColor(0.16f, 0.42f, 0.16f);
		Def.BellyColor = FLinearColor(0.78f, 0.74f, 0.42f);
		Def.MinWeightLbs = 1.5f;
		Def.MaxWeightLbs = 8.0f;
		Def.SwimSpeed = 180.f;
		Def.DetectRadius = 900.f;
		Def.BiteChance = 0.55f;
		Def.FightIntensity = 0.55f;
		Def.StaminaSeconds = 6.0f;
		Def.SpawnWeight = 1.0f;
		Def.MeshScale = FVector(1.0f, 1.8f, 0.7f);
		Def.ScorePerPound = 10.f;
		break;
	}

	return Def;
}

void UBFLFishCatalog::FillBuiltins(TArray<FBFLFishSpeciesDef>& OutSpecies)
{
	OutSpecies.Reset();
	OutSpecies.Add(MakeBuiltin(EFishSpecies::LargemouthBass));
	OutSpecies.Add(MakeBuiltin(EFishSpecies::SmallmouthBass));
	OutSpecies.Add(MakeBuiltin(EFishSpecies::Crappie));
	OutSpecies.Add(MakeBuiltin(EFishSpecies::TrophyBass));
}

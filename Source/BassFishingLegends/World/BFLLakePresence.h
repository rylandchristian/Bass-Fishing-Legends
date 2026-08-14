#pragma once

#include "CoreMinimal.h"

/** One placed actor, enough to decide whether the Lake is already authored. */
struct FBFLPlacedActor
{
	TArray<FName> Tags;
	FString Name;
	FString ClassName;
	FVector Location = FVector::ZeroVector;
};

struct FBFLLakeLayout
{
	FVector Center = FVector::ZeroVector;
	float WaterHeight = 0.f;
	bool bHasWaterSurface = false;
};

/**
 * Auto-build vs authored Lake. A WaterSurface tag or Water Body skips the
 * primitive lake. Gameplay height is that actor's Z, once — not waves.
 */
namespace BFLLakePresence
{
	inline bool HasTag(const FBFLPlacedActor& Actor, const FName Tag)
	{
		return Actor.Tags.Contains(Tag);
	}

	inline bool IsWaterSurface(const FBFLPlacedActor& Actor)
	{
		// An island sits in the Lake. It is not the water surface and must not
		// move the Z-plane center.
		if (Actor.ClassName.Contains(TEXT("WaterBodyIsland")) || Actor.Name.Contains(TEXT("WaterBodyIsland")))
		{
			return false;
		}
		if (HasTag(Actor, FName(TEXT("WaterSurface"))))
		{
			return true;
		}
		if (Actor.Name.Contains(TEXT("WaterSurface")))
		{
			return true;
		}
		return Actor.ClassName.Contains(TEXT("WaterBody"));
	}

	inline bool ShouldSkipForGenerated(const FBFLPlacedActor& Actor)
	{
		return HasTag(Actor, FName(TEXT("BFL_Generated")));
	}

	inline bool ShouldAutoBuild(const TArray<FBFLPlacedActor>& Actors, bool bSkipIfWaterExists)
	{
		if (!bSkipIfWaterExists)
		{
			return true;
		}
		for (const FBFLPlacedActor& Actor : Actors)
		{
			if (IsWaterSurface(Actor) || ShouldSkipForGenerated(Actor))
			{
				return false;
			}
		}
		return true;
	}

	inline FBFLLakeLayout Resolve(const TArray<FBFLPlacedActor>& Actors, const FVector& DefaultCenter, float DefaultWaterHeight)
	{
		FBFLLakeLayout Layout;
		Layout.Center = DefaultCenter;
		Layout.WaterHeight = DefaultWaterHeight;

		for (const FBFLPlacedActor& Actor : Actors)
		{
			if (!IsWaterSurface(Actor))
			{
				continue;
			}
			Layout.bHasWaterSurface = true;
			Layout.Center = FVector(Actor.Location.X, Actor.Location.Y, 0.f);
			Layout.WaterHeight = Actor.Location.Z;
			return Layout;
		}
		return Layout;
	}
}

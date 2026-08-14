#pragma once

#include "CoreMinimal.h"

/** Authored bass vs placeholder primitives: scale without stretch, hide the cone tail. */
namespace BFLFishLook
{
	// Placeholder: catalog stretch. Authored: uniform size from MeshScale.X (body size, not length stretch).
	inline FVector BodyMeshScale(bool bHasAuthoredBody, const FVector& SpeciesMeshScale)
	{
		if (!bHasAuthoredBody)
		{
			return SpeciesMeshScale;
		}
		const float Size = FMath::Abs(SpeciesMeshScale.X);
		return FVector(Size);
	}

	inline bool ShouldShowTail(bool bHasAuthoredBody)
	{
		return !bHasAuthoredBody;
	}
}

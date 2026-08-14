#pragma once

#include "CoreMinimal.h"

/** Charge-cast start: Rod tip, then an override, then the Boat. */
namespace BFLCastOrigin
{
	inline FVector Resolve(const FVector* RodTipWorld, const FVector* OverrideWorld, const FVector& OwnerFallback)
	{
		if (RodTipWorld)
		{
			return *RodTipWorld;
		}
		if (OverrideWorld)
		{
			return *OverrideWorld;
		}
		return OwnerFallback;
	}
}

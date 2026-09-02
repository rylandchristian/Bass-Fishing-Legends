#pragma once

#include "CoreMinimal.h"

/**
 * One tick of a fight: tension, reel, land, or snap.
 * Pure math — no world, no actors. TickFighting and tests call the same step.
 *
 * Pump: reel while tension is not red; ease off in red. Reeling in red
 * still adds overreel tension, but does not advance reel.
 */
struct FBFLFightRates
{
	/** Tension added per second while reeling (on top of the fish's pull). */
	float ReelTensionRate = 0.28f;

	/** Tension removed per second while easing off. */
	float TensionDecayRate = 0.30f;

	float IdleTensionFloor = 0.08f;

	/** Extra tension per second, scaled by how far into red you keep reeling. */
	float OverreelPenalty = 0.45f;

	/** Yellow band starts here. Reel through this. */
	float YellowTension = 0.55f;

	/** Red band / overreel starts here. Ease off. Reel does not advance here. */
	float RedTension = 0.78f;

	/** How hard the fish's pull becomes tension. */
	float FishPullToTension = 0.16f;

	/** Reel lost per second while easing off (slow leak). */
	float ReelLeakRate = 0.015f;

	/** Scales reel gained while reeling in green/yellow. Higher = faster land. */
	float ReelGainScale = 2.4f;

	float StartTension = 0.22f;
};

struct FBFLFightState
{
	float Tension = 0.22f;
	float Reel = 0.f;
	float PulsePhase = 0.f;
};

enum class EBFLFightOutcome : uint8
{
	Continue,
	Land,
	Snap
};

namespace BFLFightMath
{
	inline float FishPull(float FightIntensity, float PulsePhase)
	{
		const float Pulse = 0.5f + 0.5f * FMath::Sin(PulsePhase);
		const float Surge = 0.5f + 0.5f * FMath::Sin(PulsePhase * 0.37f + 1.1f);
		return FightIntensity * (0.35f + 0.65f * Pulse * Surge);
	}

	/** Advance one fight tick. Mutates State. */
	inline EBFLFightOutcome Tick(
		FBFLFightState& State,
		const bool bReeling,
		const float FightIntensity,
		const float StaminaSeconds,
		const float DeltaTime,
		const FBFLFightRates& Rates)
	{
		State.PulsePhase += DeltaTime * (1.6f + FightIntensity);
		const float Pull = FishPull(FightIntensity, State.PulsePhase);

		float TensionDelta = Pull * Rates.FishPullToTension;
		if (bReeling)
		{
			TensionDelta += Rates.ReelTensionRate;
			if (State.Tension > Rates.RedTension)
			{
				TensionDelta += Rates.OverreelPenalty * (State.Tension - Rates.RedTension);
			}
		}
		else
		{
			TensionDelta -= Rates.TensionDecayRate;
		}

		State.Tension = FMath::Clamp(State.Tension + TensionDelta * DeltaTime, Rates.IdleTensionFloor, 1.f);

		if (bReeling)
		{
			if (State.Tension < Rates.RedTension)
			{
				const float Efficiency = FMath::Clamp(1.15f - State.Tension * 0.85f, 0.25f, 1.f);
				const float Need = FMath::Max(StaminaSeconds, 1.f);
				State.Reel = FMath::Clamp(
					State.Reel + (Efficiency * Rates.ReelGainScale * DeltaTime) / Need,
					0.f,
					1.f);
			}
		}
		else
		{
			State.Reel = FMath::Clamp(State.Reel - Rates.ReelLeakRate * DeltaTime, 0.f, 1.f);
		}

		if (State.Tension >= 1.f)
		{
			return EBFLFightOutcome::Snap;
		}
		if (State.Reel >= 1.f)
		{
			return EBFLFightOutcome::Land;
		}
		return EBFLFightOutcome::Continue;
	}
}

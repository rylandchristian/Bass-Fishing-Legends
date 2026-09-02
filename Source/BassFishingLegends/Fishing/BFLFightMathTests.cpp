#include "Misc/AutomationTest.h"
#include "Fishing/BFLFightMath.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	// Builtin species fight numbers from UBFLFishCatalog::MakeBuiltin.
	constexpr float LargemouthIntensity = 0.55f;
	constexpr float LargemouthStamina = 6.0f;
	constexpr float CrappieIntensity = 0.28f;
	constexpr float CrappieStamina = 3.2f;
	constexpr float TrophyIntensity = 0.95f;
	constexpr float TrophyStamina = 11.0f;

	float SecondsToLandWithPump(const float Intensity, const float Stamina, const FBFLFightRates& Rates)
	{
		FBFLFightState State;
		State.Tension = Rates.StartTension;
		bool bReeling = true;
		float Time = 0.f;
		const float Dt = 1.f / 60.f;
		const float MaxSeconds = 40.f;
		while (Time < MaxSeconds)
		{
			if (State.Tension >= Rates.RedTension)
			{
				bReeling = false;
			}
			else if (State.Tension < Rates.RedTension - 0.04f)
			{
				bReeling = true;
			}

			const EBFLFightOutcome Out = BFLFightMath::Tick(State, bReeling, Intensity, Stamina, Dt, Rates);
			Time += Dt;
			if (Out == EBFLFightOutcome::Land)
			{
				return Time;
			}
			if (Out == EBFLFightOutcome::Snap)
			{
				return -1.f;
			}
		}
		return -1.f;
	}

	float SecondsToSnapIfHeld(const float Intensity, const float Stamina, const FBFLFightRates& Rates)
	{
		FBFLFightState State;
		State.Tension = Rates.StartTension;
		float Time = 0.f;
		const float Dt = 1.f / 60.f;
		const float MaxSeconds = 40.f;
		while (Time < MaxSeconds)
		{
			const EBFLFightOutcome Out = BFLFightMath::Tick(State, true, Intensity, Stamina, Dt, Rates);
			Time += Dt;
			if (Out == EBFLFightOutcome::Snap)
			{
				return Time;
			}
			if (Out == EBFLFightOutcome::Land)
			{
				return -1.f;
			}
		}
		return -1.f;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLFight_PumpLandsLargemouthInFiveToTenSeconds,
	"BFL.Fight.PumpLandsLargemouthInFiveToTenSeconds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLFight_PumpLandsLargemouthInFiveToTenSeconds::RunTest(const FString& Parameters)
{
	const float Seconds = SecondsToLandWithPump(LargemouthIntensity, LargemouthStamina, FBFLFightRates());
	TestTrue(TEXT("Pump lands a largemouth"), Seconds > 0.f);
	TestTrue(TEXT("Land takes at least 5 seconds"), Seconds >= 5.f);
	TestTrue(TEXT("Land takes at most 10 seconds"), Seconds <= 10.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLFight_HoldingReelSnapsLargemouth,
	"BFL.Fight.HoldingReelSnapsLargemouth",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLFight_HoldingReelSnapsLargemouth::RunTest(const FString& Parameters)
{
	const float Seconds = SecondsToSnapIfHeld(LargemouthIntensity, LargemouthStamina, FBFLFightRates());
	TestTrue(TEXT("Holding reel snaps before a land"), Seconds > 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLFight_HoldingReelSnapsCrappie,
	"BFL.Fight.HoldingReelSnapsCrappie",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLFight_HoldingReelSnapsCrappie::RunTest(const FString& Parameters)
{
	const float Seconds = SecondsToSnapIfHeld(CrappieIntensity, CrappieStamina, FBFLFightRates());
	TestTrue(TEXT("Holding reel snaps a crappie before a land"), Seconds > 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLFight_PumpLandsTrophy,
	"BFL.Fight.PumpLandsTrophy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLFight_PumpLandsTrophy::RunTest(const FString& Parameters)
{
	const float Seconds = SecondsToLandWithPump(TrophyIntensity, TrophyStamina, FBFLFightRates());
	TestTrue(TEXT("Pump lands a trophy"), Seconds > 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLFight_CrappieLandsFasterThanLargemouth,
	"BFL.Fight.CrappieLandsFasterThanLargemouth",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLFight_CrappieLandsFasterThanLargemouth::RunTest(const FString& Parameters)
{
	const FBFLFightRates Rates;
	const float Crappie = SecondsToLandWithPump(CrappieIntensity, CrappieStamina, Rates);
	const float Bass = SecondsToLandWithPump(LargemouthIntensity, LargemouthStamina, Rates);
	TestTrue(TEXT("Crappie lands"), Crappie > 0.f);
	TestTrue(TEXT("Largemouth lands"), Bass > 0.f);
	TestTrue(TEXT("Crappie is shorter"), Crappie < Bass);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLFight_EaseLeakIsSlow,
	"BFL.Fight.EaseLeakIsSlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLFight_EaseLeakIsSlow::RunTest(const FString& Parameters)
{
	const FBFLFightRates Rates;
	FBFLFightState State;
	State.Tension = 0.5f;
	State.Reel = 0.5f;
	BFLFightMath::Tick(State, false, LargemouthIntensity, LargemouthStamina, 1.f, Rates);
	const float Lost = 0.5f - State.Reel;
	TestTrue(TEXT("Ease leaks some reel"), Lost > 0.f);
	TestTrue(TEXT("Leak is far slower than the old 0.06/s"), Lost < 0.03f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLFight_OverreelOnlyInRed,
	"BFL.Fight.OverreelOnlyInRed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLFight_OverreelOnlyInRed::RunTest(const FString& Parameters)
{
	const FBFLFightRates Rates;
	FBFLFightRates NoPenalty = Rates;
	NoPenalty.OverreelPenalty = 0.f;

	FBFLFightState Yellow;
	Yellow.Tension = 0.70f;
	FBFLFightState YellowNoPen = Yellow;
	BFLFightMath::Tick(Yellow, true, LargemouthIntensity, LargemouthStamina, 0.05f, Rates);
	BFLFightMath::Tick(YellowNoPen, true, LargemouthIntensity, LargemouthStamina, 0.05f, NoPenalty);
	TestEqual(TEXT("Yellow reeling is not overreel"), Yellow.Tension, YellowNoPen.Tension);

	FBFLFightState Red;
	Red.Tension = 0.90f;
	FBFLFightState RedNoPen = Red;
	BFLFightMath::Tick(Red, true, LargemouthIntensity, LargemouthStamina, 0.05f, Rates);
	BFLFightMath::Tick(RedNoPen, true, LargemouthIntensity, LargemouthStamina, 0.05f, NoPenalty);
	TestTrue(TEXT("Red reeling adds overreel"), Red.Tension > RedNoPen.Tension);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLFight_ReelingInRedDoesNotAdvanceReel,
	"BFL.Fight.ReelingInRedDoesNotAdvanceReel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLFight_ReelingInRedDoesNotAdvanceReel::RunTest(const FString& Parameters)
{
	const FBFLFightRates Rates;
	FBFLFightState State;
	State.Tension = 0.90f;
	State.Reel = 0.50f;
	BFLFightMath::Tick(State, true, LargemouthIntensity, LargemouthStamina, 0.25f, Rates);
	TestEqual(TEXT("Reel does not advance in red"), State.Reel, 0.50f);
	TestTrue(TEXT("Tension still climbs in red"), State.Tension > 0.90f);
	return true;
}

#endif

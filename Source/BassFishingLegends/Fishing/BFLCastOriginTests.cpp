#include "Misc/AutomationTest.h"
#include "Fishing/BFLCastOrigin.h"
#include "Game/BFLStatics.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLCast_OriginIsRodTip,
	"BFL.Look.ChargeCastLeavesTheRodTip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLCast_OriginIsRodTip::RunTest(const FString& Parameters)
{
	const FVector RodTip(120.f, 40.f, 80.f);
	const FVector Override(0.f, 0.f, 0.f);
	const FVector Boat(0.f, 0.f, 22.f);
	const FVector Origin = BFLCastOrigin::Resolve(&RodTip, &Override, Boat);
	TestEqual(TEXT("Charge-cast starts at the Rod tip"), Origin, RodTip);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLCast_OriginIgnoresBoatWhenRodTipExists,
	"BFL.Look.ChargeCastDoesNotUseBoatWhenRodTipExists",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLCast_OriginIgnoresBoatWhenRodTipExists::RunTest(const FString& Parameters)
{
	const FVector RodTip(50.f, 18.f, 75.f);
	const FVector Boat(10.f, 10.f, 22.f);
	const FVector Origin = BFLCastOrigin::Resolve(&RodTip, nullptr, Boat);
	TestEqual(TEXT("Boat location is not the Charge-cast origin"), Origin, RodTip);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLWater_HitUsesContractHeightNotAWave,
	"BFL.Look.LureHitUsesContractWaterHeight",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLWater_HitUsesContractHeightNotAWave::RunTest(const FString& Parameters)
{
	const float ContractZ = 0.f;
	const float WaveZ = 14.f;
	FVector Hit = FVector::ZeroVector;
	const bool bHit = UBFLStatics::PredictWaterHit(
		FVector(0.f, 0.f, 80.f),
		FVector(200.f, 0.f, -400.f),
		-980.f,
		ContractZ,
		4.f,
		Hit);
	TestTrue(TEXT("Lure hits the water plane"), bHit);
	TestTrue(TEXT("Hit Z is the contract height, not a wave"), FMath::IsNearlyEqual(Hit.Z, static_cast<double>(ContractZ)));
	TestTrue(TEXT("Wave height is not the hit"), !FMath::IsNearlyEqual(Hit.Z, static_cast<double>(WaveZ)));
	return true;
}

#endif

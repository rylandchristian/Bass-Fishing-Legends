#include "Misc/AutomationTest.h"
#include "World/BFLLakePresence.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	FBFLPlacedActor MakeActor(const TCHAR* Name, const FName Tag = NAME_None, const FVector& Location = FVector::ZeroVector, const TCHAR* ClassName = TEXT("StaticMeshActor"))
	{
		FBFLPlacedActor Actor;
		Actor.Name = Name;
		Actor.ClassName = ClassName;
		Actor.Location = Location;
		if (!Tag.IsNone())
		{
			Actor.Tags.Add(Tag);
		}
		return Actor;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLLake_NoWaterStillAutoBuilds,
	"BFL.Look.NoWaterSurfaceStillAutoBuilds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLLake_NoWaterStillAutoBuilds::RunTest(const FString& Parameters)
{
	TArray<FBFLPlacedActor> Actors;
	Actors.Add(MakeActor(TEXT("PlayerStart")));
	TestTrue(TEXT("Blank map still auto-builds"), BFLLakePresence::ShouldAutoBuild(Actors, true));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLLake_TaggedWaterSkipsAutoBuild,
	"BFL.Look.TaggedWaterSkipsAutoBuild",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLLake_TaggedWaterSkipsAutoBuild::RunTest(const FString& Parameters)
{
	TArray<FBFLPlacedActor> Actors;
	Actors.Add(MakeActor(TEXT("WaterBodyLake_0"), FName(TEXT("WaterSurface")), FVector(100.f, 200.f, 50.f)));
	TestFalse(TEXT("Tagged water skips primitives"), BFLLakePresence::ShouldAutoBuild(Actors, true));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLLake_WaterBodyClassSkipsAutoBuild,
	"BFL.Look.WaterBodyClassSkipsAutoBuild",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLLake_WaterBodyClassSkipsAutoBuild::RunTest(const FString& Parameters)
{
	TArray<FBFLPlacedActor> Actors;
	Actors.Add(MakeActor(TEXT("Lake"), NAME_None, FVector::ZeroVector, TEXT("WaterBodyLake")));
	TestFalse(TEXT("Water Body Lake skips primitives"), BFLLakePresence::ShouldAutoBuild(Actors, true));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLLake_SkipFlagOffAlwaysBuilds,
	"BFL.Look.SkipFlagOffAlwaysBuilds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLLake_SkipFlagOffAlwaysBuilds::RunTest(const FString& Parameters)
{
	TArray<FBFLPlacedActor> Actors;
	Actors.Add(MakeActor(TEXT("WaterSurface"), FName(TEXT("WaterSurface"))));
	TestTrue(TEXT("Designer can force auto-build"), BFLLakePresence::ShouldAutoBuild(Actors, false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLLake_TaggedWaterSetsHeightFromActorZ,
	"BFL.Look.TaggedWaterSetsHeightFromActorZ",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLLake_TaggedWaterSetsHeightFromActorZ::RunTest(const FString& Parameters)
{
	TArray<FBFLPlacedActor> Actors;
	Actors.Add(MakeActor(TEXT("WaterSurface"), FName(TEXT("WaterSurface")), FVector(80.f, -40.f, 125.f)));
	const FBFLLakeLayout Layout = BFLLakePresence::Resolve(Actors, FVector::ZeroVector, 0.f);
	TestTrue(TEXT("Found a water surface"), Layout.bHasWaterSurface);
	TestTrue(TEXT("Water height is the actor Z"), FMath::IsNearlyEqual(Layout.WaterHeight, 125.f));
	TestTrue(TEXT("Lake center X"), FMath::IsNearlyEqual(Layout.Center.X, 80.0));
	TestTrue(TEXT("Lake center Y"), FMath::IsNearlyEqual(Layout.Center.Y, -40.0));
	TestTrue(TEXT("Lake center stays on the Z-plane"), FMath::IsNearlyEqual(Layout.Center.Z, 0.0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLLake_GeneratedPropsDoNotMoveWaterHeight,
	"BFL.Look.GeneratedPropsDoNotMoveWaterHeight",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLLake_GeneratedPropsDoNotMoveWaterHeight::RunTest(const FString& Parameters)
{
	TArray<FBFLPlacedActor> Actors;
	Actors.Add(MakeActor(TEXT("Rock_0"), FName(TEXT("BFL_Generated")), FVector(0.f, 0.f, 18.f)));
	const FBFLLakeLayout Layout = BFLLakePresence::Resolve(Actors, FVector(10.f, 20.f, 0.f), 0.f);
	TestFalse(TEXT("A generated rock is not the water surface"), Layout.bHasWaterSurface);
	TestEqual(TEXT("Contract water height is unchanged"), Layout.WaterHeight, 0.f);
	TestEqual(TEXT("Lake center is unchanged"), Layout.Center, FVector(10.f, 20.f, 0.f));
	TestFalse(TEXT("Generated leftovers still skip a second primitive lake"), BFLLakePresence::ShouldAutoBuild(Actors, true));
	return true;
}

#endif

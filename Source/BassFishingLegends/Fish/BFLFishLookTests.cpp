#include "Misc/AutomationTest.h"
#include "Fish/BFLFishLook.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLLook_AuthoredFishUsesUniformScale,
	"BFL.Look.AuthoredFishUsesUniformScale",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLLook_AuthoredFishUsesUniformScale::RunTest(const FString& Parameters)
{
	const FVector TrophyScale(1.45f, 2.6f, 1.0f);
	const FVector Trophy = BFLFishLook::BodyMeshScale(true, TrophyScale);
	TestEqual(TEXT("Trophy uses MeshScale.X, not the placeholder length stretch"), Trophy, FVector(1.45f));

	const FVector BassScale(1.0f, 1.8f, 0.7f);
	const FVector Bass = BFLFishLook::BodyMeshScale(true, BassScale);
	TestEqual(TEXT("Largemouth authored size stays 1"), Bass, FVector(1.0f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLLook_PlaceholderFishKeepsStretch,
	"BFL.Look.PlaceholderFishKeepsStretch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLLook_PlaceholderFishKeepsStretch::RunTest(const FString& Parameters)
{
	const FVector SpeciesScale(1.0f, 1.8f, 0.7f);
	const FVector Scale = BFLFishLook::BodyMeshScale(false, SpeciesScale);
	TestEqual(TEXT("Placeholder body keeps the catalog stretch"), Scale, SpeciesScale);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLLook_AuthoredFishHidesTail,
	"BFL.Look.AuthoredFishHidesTail",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLLook_AuthoredFishHidesTail::RunTest(const FString& Parameters)
{
	TestFalse(TEXT("Authored body does not show the cone tail"), BFLFishLook::ShouldShowTail(true));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLLook_PlaceholderFishShowsTail,
	"BFL.Look.PlaceholderFishShowsTail",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLLook_PlaceholderFishShowsTail::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Placeholder body still shows the cone tail"), BFLFishLook::ShouldShowTail(false));
	return true;
}

#endif

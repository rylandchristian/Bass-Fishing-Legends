#include "Misc/AutomationTest.h"
#include "Game/BFLAssignedMesh.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLAssignedMesh_UnsetUsesPlaceholder,
	"BFL.Look.UnsetMeshUsesPlaceholder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLAssignedMesh_UnsetUsesPlaceholder::RunTest(const FString& Parameters)
{
	int32 Placeholder = 1;
	int32* Chosen = BFLAssignedMesh::Keep<int32>(nullptr, &Placeholder);
	TestEqual(TEXT("Unset mesh takes the placeholder"), Chosen, &Placeholder);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLAssignedMesh_AssignedIsNotReplaced,
	"BFL.Look.AssignedMeshIsNotReplaced",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLAssignedMesh_AssignedIsNotReplaced::RunTest(const FString& Parameters)
{
	int32 Authored = 1;
	int32 Placeholder = 2;
	int32* Chosen = BFLAssignedMesh::Keep<int32>(&Authored, &Placeholder);
	TestEqual(TEXT("Authored mesh is kept"), Chosen, &Authored);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLAssignedMesh_BothMissingStaysUnset,
	"BFL.Look.BothMissingStaysUnset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLAssignedMesh_BothMissingStaysUnset::RunTest(const FString& Parameters)
{
	int32* Chosen = BFLAssignedMesh::Keep<int32>(nullptr, nullptr);
	TestNull(TEXT("No mesh stays unset"), Chosen);
	return true;
}

#endif

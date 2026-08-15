#include "Misc/AutomationTest.h"
#include "World/BFLWaterSurfaceDraw.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	FBFLWaterMeshView MakeMesh(const TCHAR* ComponentClass, bool bEnabled, int32 TilesX, int32 TilesY, int32 Vertices = 0)
	{
		FBFLWaterMeshView Mesh;
		Mesh.OwnerClassName = TEXT("WaterZone");
		Mesh.ComponentClassName = ComponentClass;
		Mesh.bEnabled = bEnabled;
		Mesh.TileCountX = TilesX;
		Mesh.TileCountY = TilesY;
		Mesh.VertexCount = Vertices;
		return Mesh;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLLook_WaterBodyIsNotAWaterZone,
	"BFL.Look.WaterBodyIsNotAWaterZone",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLLook_WaterBodyIsNotAWaterZone::RunTest(const FString& Parameters)
{
	TestFalse(TEXT("A Water Body Lake is not the Water Zone"), BFLWaterSurfaceDraw::IsWaterZoneClass(TEXT("WaterBodyLake")));
	TestFalse(TEXT("An island is not the Water Zone"), BFLWaterSurfaceDraw::IsWaterZoneClass(TEXT("WaterBodyIsland")));
	TestTrue(TEXT("Water Zone class is recognized"), BFLWaterSurfaceDraw::IsWaterZoneClass(TEXT("WaterZone")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLLook_DisabledWaterMeshIsNotDrawable,
	"BFL.Look.DisabledWaterMeshIsNotDrawable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLLook_DisabledWaterMeshIsNotDrawable::RunTest(const FString& Parameters)
{
	const FBFLWaterMeshView Mesh = MakeMesh(TEXT("WaterMeshComponent"), false, 8, 8);
	TestFalse(TEXT("A disabled Water Zone mesh is not drawable"), BFLWaterSurfaceDraw::IsDrawable(Mesh));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLLook_EnabledWaterMeshWithoutTilesIsNotDrawable,
	"BFL.Look.EnabledWaterMeshWithoutTilesIsNotDrawable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLLook_EnabledWaterMeshWithoutTilesIsNotDrawable::RunTest(const FString& Parameters)
{
	const FBFLWaterMeshView Mesh = MakeMesh(TEXT("WaterMeshComponent"), true, 0, 0);
	TestFalse(TEXT("Enabled but never built is not drawable"), BFLWaterSurfaceDraw::IsDrawable(Mesh));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLLook_EnabledWaterMeshWithTilesIsDrawable,
	"BFL.Look.EnabledWaterMeshWithTilesIsDrawable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLLook_EnabledWaterMeshWithTilesIsDrawable::RunTest(const FString& Parameters)
{
	const FBFLWaterMeshView Mesh = MakeMesh(TEXT("WaterMeshComponent"), true, 8, 8);
	TestTrue(TEXT("Enabled Water Zone mesh with tiles is drawable"), BFLWaterSurfaceDraw::IsDrawable(Mesh));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLLook_EnabledWaterMeshWithVerticesIsDrawable,
	"BFL.Look.EnabledWaterMeshWithVerticesIsDrawable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FBFLLook_EnabledWaterMeshWithVerticesIsDrawable::RunTest(const FString& Parameters)
{
	const FBFLWaterMeshView Mesh = MakeMesh(TEXT("WaterMeshComponent"), true, 0, 0, 64);
	TestTrue(TEXT("Enabled Water Zone mesh with vertices is drawable"), BFLWaterSurfaceDraw::IsDrawable(Mesh));
	return true;
}

#if WITH_EDITOR

#include "Editor.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "Tests/AutomationCommon.h"
#include "UObject/UnrealType.h"

namespace
{
	FString ClassChain(const UObject* Object)
	{
		FString Names;
		for (const UClass* Class = Object->GetClass(); Class; Class = Class->GetSuperClass())
		{
			if (!Names.IsEmpty())
			{
				Names += TEXT(" ");
			}
			Names += Class->GetName();
		}
		return Names;
	}

	bool ReadIsEnabled(UActorComponent* Component)
	{
		UFunction* Function = Component->FindFunction(FName(TEXT("IsEnabled")));
		if (!Function)
		{
			return false;
		}
		struct FIsEnabledParams
		{
			bool ReturnValue = false;
		};
		FIsEnabledParams Params;
		Component->ProcessEvent(Function, &Params);
		return Params.ReturnValue;
	}

	FIntPoint ReadQuadTreeResolution(const UActorComponent* Component)
	{
		const FStructProperty* Property = FindFProperty<FStructProperty>(Component->GetClass(), TEXT("QuadTreeResolution"));
		if (!Property)
		{
			return FIntPoint::ZeroValue;
		}
		return *Property->ContainerPtrToValuePtr<FIntPoint>(Component);
	}

	bool FindWaterZoneMesh(UWorld* World, FBFLWaterMeshView& OutMesh)
	{
		if (!World)
		{
			return false;
		}

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (!BFLWaterSurfaceDraw::IsWaterZoneClass(ClassChain(*It)))
			{
				continue;
			}

			for (UActorComponent* Component : It->GetComponents())
			{
				if (!Component || !BFLWaterSurfaceDraw::IsWaterMeshClass(ClassChain(Component)))
				{
					continue;
				}

				OutMesh.OwnerClassName = ClassChain(*It);
				OutMesh.ComponentClassName = ClassChain(Component);
				OutMesh.bEnabled = ReadIsEnabled(Component);
				const FIntPoint Tiles = ReadQuadTreeResolution(Component);
				OutMesh.TileCountX = Tiles.X;
				OutMesh.TileCountY = Tiles.Y;
				return true;
			}
		}
		return false;
	}

	class FBFLAssertAuthoredLakeWaterMesh : public IAutomationLatentCommand
	{
	public:
		explicit FBFLAssertAuthoredLakeWaterMesh(FAutomationTestBase* InTest)
			: Test(InTest)
		{
		}

		virtual bool Update() override
		{
			UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
			FBFLWaterMeshView Mesh;
			if (!FindWaterZoneMesh(World, Mesh))
			{
				Test->AddError(TEXT("Authored Lake has no Water Zone mesh"));
				return true;
			}

			Test->TestTrue(TEXT("Water Zone mesh is enabled"), Mesh.bEnabled);
			Test->TestTrue(
				TEXT("Water Zone mesh has tiles"),
				Mesh.TileCountX > 0 && Mesh.TileCountY > 0);
			Test->TestTrue(TEXT("Water Zone mesh is drawable"), BFLWaterSurfaceDraw::IsDrawable(Mesh));
			return true;
		}

	private:
		FAutomationTestBase* Test = nullptr;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBFLLook_AuthoredLakeHasDrawableWaterSurface,
	"BFL.Look.AuthoredLakeHasDrawableWaterSurface",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::NonNullRHI | EAutomationTestFlags::ProductFilter)

bool FBFLLook_AuthoredLakeHasDrawableWaterSurface::RunTest(const FString& Parameters)
{
	const bool bLoadAsTemplate = false;
	const bool bShowProgress = false;
	if (!FEditorFileUtils::LoadMap(TEXT("/Game/Maps/Lake"), bLoadAsTemplate, bShowProgress))
	{
		AddError(TEXT("Could not load the authored Lake"));
		return false;
	}

	// Water Zone tiles rebuild on tick. Give the mesh one beat after load.
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FBFLAssertAuthoredLakeWaterMesh(this));
	return true;
}

#endif // WITH_EDITOR

#endif

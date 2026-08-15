#pragma once

#include "CoreMinimal.h"

/** One Water Zone mesh, enough to decide whether the Water surface can draw. */
struct FBFLWaterMeshView
{
	FString OwnerClassName;
	FString ComponentClassName;
	bool bEnabled = false;
	int32 TileCountX = 0;
	int32 TileCountY = 0;
	int32 VertexCount = 0;
};

/**
 * Water Zone mesh vs other water actors. The game module does not include the
 * Water plugin — class-name walk only, same spirit as BFLLakePresence.
 */
namespace BFLWaterSurfaceDraw
{
	inline bool IsWaterZoneClass(const FString& ClassName)
	{
		return ClassName.Contains(TEXT("WaterZone"));
	}

	inline bool IsWaterMeshClass(const FString& ClassName)
	{
		return ClassName.Contains(TEXT("WaterMesh"));
	}

	inline bool IsDrawable(const FBFLWaterMeshView& Mesh)
	{
		if (!IsWaterMeshClass(Mesh.ComponentClassName))
		{
			return false;
		}
		if (!Mesh.bEnabled)
		{
			return false;
		}
		const bool bHasTiles = Mesh.TileCountX > 0 && Mesh.TileCountY > 0;
		return bHasTiles || Mesh.VertexCount > 0;
	}
}

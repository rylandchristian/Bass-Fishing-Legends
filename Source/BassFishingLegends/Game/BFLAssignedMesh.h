#pragma once

/**
 * Placeholder mesh assignment: keep an already-set mesh.
 * Unset meshes still take the engine primitive so a blank project plays.
 */
namespace BFLAssignedMesh
{
	template <typename T>
	T* Keep(T* Current, T* Placeholder)
	{
		return Current ? Current : Placeholder;
	}
}

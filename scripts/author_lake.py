"""Author /Game/Maps/Lake: Water Body, irregular shore, one island, basin mesh.

Run from UnrealEditor-Cmd with -ExecutePythonScript. Does not buy or invent
Boat / Angler / bass pack meshes.
"""

from __future__ import annotations

import math
import os
import sys

# ExecutePythonScript does not put this file's directory on sys.path.
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from lake_waterline import (
    ISLAND_CENTER,
    WATER_HEIGHT,
    island_cutout_points,
    island_radius,
    shore_radius,
    water_points,
)

import unreal


MAP_HALF = 6400.0
GRID_STEP = 64.0
MAP_PATH = "/Game/Maps/Lake"
MESH_PATH = "/Game/Meshes/SM_LakeBasin"


def height_at(x: float, y: float) -> float:
    island_dx = x - ISLAND_CENTER[0]
    island_dy = y - ISLAND_CENTER[1]
    island_dist = math.hypot(island_dx, island_dy)
    island_ang = math.atan2(island_dy, island_dx)
    this_island_r = island_radius(island_ang)

    angle = math.atan2(y, x)
    r_shore = shore_radius(angle)
    radius = math.hypot(x, y)

    bank_noise = 10.0 * math.sin(x * 0.004) * math.cos(y * 0.0035)

    if island_dist < this_island_r:
        t = 1.0 - island_dist / max(this_island_r, 1.0)
        return 28.0 + 110.0 * t * t + bank_noise * 0.35

    if radius >= r_shore:
        over = radius - r_shore
        return 32.0 + min(over * 0.085, 190.0) + bank_noise

    t = radius / max(r_shore, 1.0)
    bed = -90.0 - 230.0 * (1.0 - t * t)
    if t > 0.80:
        blend = (t - 0.80) / 0.20
        bed = bed * (1.0 - blend ** 0.55)
    return bed


def to_water_vectors(points: list[tuple[float, float]]) -> list[unreal.Vector]:
    return [unreal.Vector(x, y, WATER_HEIGHT) for x, y in points]


def write_basin_obj(path: str) -> None:
    xs: list[float] = []
    x = -MAP_HALF
    while x <= MAP_HALF + 0.01:
        xs.append(x)
        x += GRID_STEP
    n = len(xs)

    verts: list[tuple[float, float, float]] = []
    uvs: list[tuple[float, float]] = []
    bank_faces: list[tuple[int, int, int]] = []
    bed_faces: list[tuple[int, int, int]] = []

    for y in xs:
        for x in xs:
            z = height_at(x, y)
            verts.append((x, y, z))
            uvs.append(((x + MAP_HALF) / (MAP_HALF * 2.0), (y + MAP_HALF) / (MAP_HALF * 2.0)))

    def vid(ix: int, iy: int) -> int:
        return iy * n + ix

    for iy in range(n - 1):
        for ix in range(n - 1):
            i00 = vid(ix, iy)
            i10 = vid(ix + 1, iy)
            i01 = vid(ix, iy + 1)
            i11 = vid(ix + 1, iy + 1)
            z_mean = (verts[i00][2] + verts[i10][2] + verts[i01][2] + verts[i11][2]) * 0.25
            dest = bank_faces if z_mean >= -8.0 else bed_faces
            dest.append((i00, i10, i11))
            dest.append((i00, i11, i01))

    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="ascii") as handle:
        handle.write("# BFL authored lake basin\n")
        handle.write("o LakeBasin\n")
        for x, y, z in verts:
            handle.write(f"v {x:.3f} {y:.3f} {z:.3f}\n")
        for u, v in uvs:
            handle.write(f"vt {u:.5f} {v:.5f}\n")
        handle.write("usemtl Bank\n")
        for a, b, c in bank_faces:
            handle.write(f"f {a + 1}/{a + 1} {b + 1}/{b + 1} {c + 1}/{c + 1}\n")
        handle.write("usemtl Bed\n")
        for a, b, c in bed_faces:
            handle.write(f"f {a + 1}/{a + 1} {b + 1}/{b + 1} {c + 1}/{c + 1}\n")
    unreal.log(f"Wrote basin OBJ ({n}x{n}) to {path}")


def make_color_material(asset_name: str, package_path: str, color: unreal.LinearColor, roughness: float):
    asset_path = f"{package_path}/{asset_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        return unreal.EditorAssetLibrary.load_asset(asset_path)

    factory = unreal.MaterialFactoryNew()
    mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name, package_path, unreal.Material, factory
    )
    if not mat:
        raise RuntimeError(f"Could not create {asset_path}")

    lib = unreal.MaterialEditingLibrary
    base = lib.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -380, 0)
    base.set_editor_property("constant", color)
    lib.connect_material_property(base, "", unreal.MaterialProperty.MP_BASE_COLOR)

    rough = lib.create_material_expression(mat, unreal.MaterialExpressionConstant, -380, 160)
    rough.set_editor_property("r", roughness)
    lib.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)

    spec = lib.create_material_expression(mat, unreal.MaterialExpressionConstant, -380, 260)
    spec.set_editor_property("r", 0.08)
    lib.connect_material_property(spec, "", unreal.MaterialProperty.MP_SPECULAR)

    lib.layout_material_expressions(mat)
    lib.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset(asset_path)
    return mat


def import_basin_mesh(obj_path: str, bank_mat, bed_mat):
    if unreal.EditorAssetLibrary.does_asset_exist(MESH_PATH):
        unreal.EditorAssetLibrary.delete_asset(MESH_PATH)

    task = unreal.AssetImportTask()
    task.filename = obj_path
    task.destination_path = "/Game/Meshes"
    task.destination_name = "SM_LakeBasin"
    task.automated = True
    task.save = True
    task.replace_existing = True

    try:
        options = unreal.FbxImportUI()
        options.automated_import_should_detect_type = False
        options.import_mesh = True
        options.import_as_skeletal = False
        options.import_animations = False
        options.import_materials = False
        options.import_textures = False
        options.static_mesh_import_data.combine_meshes = True
        options.static_mesh_import_data.generate_lightmap_u_vs = True
        options.static_mesh_import_data.auto_generate_collision = False
        options.static_mesh_import_data.convert_scene = False
        options.static_mesh_import_data.force_front_x_axis = False
        options.static_mesh_import_data.normal_import_method = unreal.FBXNormalImportMethod.FBXNIM_COMPUTE_NORMALS
        task.options = options
    except Exception as exc:
        unreal.log_warning(f"FBX import UI options unavailable ({exc}); using Interchange defaults")

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh = unreal.EditorAssetLibrary.load_asset(MESH_PATH)
    if not mesh:
        raise RuntimeError(f"Import did not produce {MESH_PATH}")

    if hasattr(mesh, "set_material"):
        mesh.set_material(0, bank_mat)
        if mesh.get_num_sections(0) > 1:
            mesh.set_material(1, bed_mat)
    unreal.EditorAssetLibrary.save_asset(MESH_PATH)
    return mesh


def set_spline(spline, points) -> None:
    spline.set_closed_loop(True, False)
    spline.set_spline_points(points, unreal.SplineCoordinateSpace.LOCAL, True)
    if hasattr(spline, "k2_synchronize_and_broadcast_data_change"):
        spline.k2_synchronize_and_broadcast_data_change()


def disable_water_collision(water_actor) -> None:
    body = water_actor.get_water_body_component()
    if not body:
        return
    for component in body.get_collision_components(False):
        if hasattr(component, "set_collision_enabled"):
            component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        if hasattr(component, "set_generate_overlap_events"):
            component.set_generate_overlap_events(False)


def author_level(mesh, bank_mat, bed_mat) -> None:
    level_sys = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actor_sys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    if unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
        unreal.EditorAssetLibrary.delete_asset(MAP_PATH)

    if not level_sys.new_level(MAP_PATH):
        raise RuntimeError(f"Could not create {MAP_PATH}")

    zone = actor_sys.spawn_actor_from_class(unreal.WaterZone, unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator())
    zone.set_actor_label("WaterZone")
    zone.set_editor_property("zone_extent", unreal.Vector2D(14000.0, 14000.0))

    water = actor_sys.spawn_actor_from_class(unreal.WaterBodyLake, unreal.Vector(0.0, 0.0, WATER_HEIGHT), unreal.Rotator())
    water.set_actor_label("WaterBodyLake")
    tags = list(water.tags)
    if "WaterSurface" not in tags:
        tags.append("WaterSurface")
    water.tags = tags
    set_spline(water.get_water_spline(), to_water_vectors(water_points()))
    disable_water_collision(water)

    island = actor_sys.spawn_actor_from_class(
        unreal.WaterBodyIsland,
        unreal.Vector(ISLAND_CENTER[0], ISLAND_CENTER[1], WATER_HEIGHT),
        unreal.Rotator(),
    )
    island.set_actor_label("WaterBodyIsland")
    # Points are lake-space around the island; actor is already at the island center,
    # so feed local offsets.
    local_island = [
        unreal.Vector(x - ISLAND_CENTER[0], y - ISLAND_CENTER[1], 0.0)
        for x, y in island_cutout_points()
    ]
    set_spline(island.get_water_spline(), local_island)

    basin = actor_sys.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator())
    basin.set_actor_label("LakeBasin")
    basin.set_mobility(unreal.ComponentMobility.STATIC)
    smc = basin.static_mesh_component
    smc.set_static_mesh(mesh)
    smc.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    smc.set_material(0, bank_mat)
    smc.set_material(1, bed_mat)

    start = actor_sys.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(0.0, 0.0, WATER_HEIGHT + 40.0),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    start.set_actor_label("PlayerStart")

    if not level_sys.save_current_level():
        raise RuntimeError(f"Could not save {MAP_PATH}")
    unreal.log(f"Saved authored Lake map at {MAP_PATH}")


def main() -> int:
    project_dir = unreal.Paths.project_dir()
    obj_path = os.path.normpath(os.path.join(project_dir, "Intermediate", "AuthoredLake", "LakeBasin.obj"))
    write_basin_obj(obj_path)

    bank_mat = make_color_material(
        "M_LakeBank",
        "/Game/Materials",
        unreal.LinearColor(0.23, 0.32, 0.14, 1.0),
        0.78,
    )
    bed_mat = make_color_material(
        "M_LakeBed",
        "/Game/Materials",
        unreal.LinearColor(0.08, 0.10, 0.07, 1.0),
        0.88,
    )
    mesh = import_basin_mesh(obj_path, bank_mat, bed_mat)
    author_level(mesh, bank_mat, bed_mat)
    return 0


def _quit_editor() -> None:
    try:
        unreal.SystemLibrary.execute_console_command(None, "QUIT")
    except Exception:
        pass


try:
    code = main()
    _quit_editor()
    sys.exit(code)
except Exception as exc:
    unreal.log_error(f"author_lake failed: {exc}")
    _quit_editor()
    raise

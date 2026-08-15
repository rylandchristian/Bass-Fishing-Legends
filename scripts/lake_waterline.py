"""Lake waterline: the Water surface sits a hair inside the Basin shore.

Shared by the Unreal authoring script and the no-editor tests. This is the
only copy of the shore function.
"""

from __future__ import annotations

import math

LAKE_RADIUS = 4500.0
ISLAND_CENTER = (1575.0, -900.0)
ISLAND_RADIUS = 620.0
WATER_HEIGHT = 0.0

# Hair inside the Banks so the Water surface does not climb the grass.
# Same absolute hair at the island. The old authoring used ~1.5% / ~8%.
WATERLINE_HAIR = 15.0

# 36 / 16 samples left ~1.4 m dry coves. These counts keep a chord inside a hair.
WATER_SAMPLE_COUNT = 128
ISLAND_SAMPLE_COUNT = 64


def shore_radius(angle: float) -> float:
    return (
        LAKE_RADIUS
        + 380.0 * math.sin(3.0 * angle)
        + 220.0 * math.cos(5.0 * angle)
        + 140.0 * math.sin(7.0 * angle + 0.6)
    )


def island_radius(angle: float) -> float:
    return ISLAND_RADIUS * (0.82 + 0.18 * math.sin(2.0 * angle + 0.4))


def _ring_points(
    radius_fn,
    count: int,
    origin: tuple[float, float] = (0.0, 0.0),
) -> list[tuple[float, float]]:
    points: list[tuple[float, float]] = []
    for i in range(count):
        angle = (2.0 * math.pi * i) / count
        radius = radius_fn(angle) - WATERLINE_HAIR
        points.append(
            (
                origin[0] + math.cos(angle) * radius,
                origin[1] + math.sin(angle) * radius,
            )
        )
    return points


def water_points() -> list[tuple[float, float]]:
    """XY samples of the Water surface in lake space."""
    return _ring_points(shore_radius, WATER_SAMPLE_COUNT)


def island_cutout_points() -> list[tuple[float, float]]:
    """XY samples of the island cutout in lake space."""
    return _ring_points(island_radius, ISLAND_SAMPLE_COUNT, ISLAND_CENTER)

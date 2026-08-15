"""Waterline contract: the Water surface sits a hair inside the Basin shore.

Issue #3. Run without the editor:

    python scripts/test_lake_waterline.py
"""

from __future__ import annotations

import math
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from lake_waterline import (
    ISLAND_CENTER,
    LAKE_RADIUS,
    WATER_HEIGHT,
    island_cutout_points,
    island_radius,
    shore_radius,
    water_points,
)

# Independent bounds from the measured dry strip on the old authoring.
# Today's 1.5% inset is 60–75 cm; today's worst water chord leaves ~1.4 m.
# A hair that still does not climb the grass is well under both.
HAIR_MAX_CM = 25.0
COVE_MAX_CM = 40.0
HEADINGS = 360


def ray_polygon_radius(
    origin: tuple[float, float],
    angle: float,
    polygon: list[tuple[float, float]],
) -> float:
    """Distance from origin to the first hit on the closed polygon."""
    ox, oy = origin
    dx, dy = math.cos(angle), math.sin(angle)
    best: float | None = None
    count = len(polygon)
    for i in range(count):
        x1, y1 = polygon[i]
        x2, y2 = polygon[(i + 1) % count]
        edge_x, edge_y = x2 - x1, y2 - y1
        den = dx * edge_y - dy * edge_x
        if abs(den) < 1e-12:
            continue
        t = ((x1 - ox) * edge_y - (y1 - oy) * edge_x) / den
        u = ((x1 - ox) * dy - (y1 - oy) * dx) / den
        if t >= -1e-9 and -1e-9 <= u <= 1.0 + 1e-9:
            if best is None or t < best:
                best = t
    if best is None:
        raise AssertionError(f"ray at {angle:.4f} missed the waterline")
    return best


def gaps_along_headings(
    origin: tuple[float, float],
    radius_fn,
    polygon: list[tuple[float, float]],
    headings: int = HEADINGS,
) -> list[float]:
    gaps: list[float] = []
    for i in range(headings):
        angle = (2.0 * math.pi * i) / headings
        shore = radius_fn(angle)
        water = ray_polygon_radius(origin, angle, polygon)
        gaps.append(shore - water)
    return gaps


class WaterlineTests(unittest.TestCase):
    def test_water_sits_a_hair_inside_the_banks(self) -> None:
        polygon = water_points()
        for gap in gaps_along_headings((0.0, 0.0), shore_radius, polygon):
            self.assertGreater(gap, 0.0, "Water surface must not climb the Banks")
            self.assertLessEqual(
                gap,
                HAIR_MAX_CM,
                "gap must be a hair, not today's ~1.5% strip",
            )

    def test_coves_are_not_chorded_off(self) -> None:
        polygon = water_points()
        worst = max(gaps_along_headings((0.0, 0.0), shore_radius, polygon, headings=720))
        self.assertLess(
            worst,
            COVE_MAX_CM,
            "a water chord must not leave a cove reading as dry bed",
        )

    def test_island_cutout_follows_the_same_hair(self) -> None:
        polygon = island_cutout_points()
        for gap in gaps_along_headings(ISLAND_CENTER, island_radius, polygon):
            self.assertGreater(gap, 0.0, "island cutout must not climb the island")
            self.assertLessEqual(
                gap,
                HAIR_MAX_CM,
                "island cutout must use the same hair as the outer waterline",
            )

    def test_playable_radius_and_water_height_stay_put(self) -> None:
        self.assertEqual(LAKE_RADIUS, 4500.0)
        self.assertEqual(WATER_HEIGHT, 0.0)


if __name__ == "__main__":
    unittest.main()

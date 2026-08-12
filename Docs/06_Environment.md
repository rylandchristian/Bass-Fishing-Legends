# 06 — Environment

## What Play already builds

If the map has no actor tagged `WaterSurface`, `ABFLGameMode::BuildDefaultLake` spawns:

- A large water plane at `WaterHeight` (default Z = 0)
- A darker depth plane under it
- A brown shore ring
- Trees (cylinder + cone) just outside the water
- Grey rock spheres along the bank
- One small island with a tree
- Directional light + sky light
- `ABFLDayNightActor`

Meshes are Engine basics (`/Engine/BasicShapes/...`) tinted at runtime. Fine for a prototype, not a ship look.

---

## Build a real lake map

1. **File → New Level → Basic** (has a floor and light) **or Empty**.
2. Save as `/Game/Maps/Lake`.
3. World Settings → GameMode Override = `BFLGameMode` (or `BP_FishingGameMode`).
4. Delete the default floor if it sits at Z = 0 and fights the water plane.

### Water (fastest)

**Placeholder (already automated)** — do nothing.

**Simple plane you control:**

1. Place a **Plane** (`Place Actors → Shapes → Plane`).
2. Scale ≈ `90, 90, 1` for a ~9000 unit lake.
3. Location Z = `0`.
4. Create **Material** `/Game/Materials/M_Water`:
   - Blend Mode = Translucent (or Opaque for cheapest)
   - Base Color = dark teal `(0.02, 0.12, 0.22)`
   - Roughness = `0.05`
   - Optional: panner on a normal map for cheap ripples
5. Apply `M_Water`.
6. Actor tag = `WaterSurface` (Details → Tags). This stops auto-build from adding a second plane.
7. Match **Project Settings → Bass Fishing Legends → Lake Radius** to your plane.

**UE Water plugin (prettier):**

1. **Edit → Plugins** → enable **Water** and **Landmass**. Restart.
2. Place **Water Body Lake**.
3. Sculpt the shoreline with the lake spline.
4. Tag the water body `WaterSurface`.
5. Set `Water Height` in Project Settings to the lake’s Z.
6. Collision: casts currently hit by **Z-plane test**, not the water collision channel. Keep `WaterHeight` honest and you do not need a custom channel.

### Boat mesh

1. Import or model a small skiff into `/Game/Meshes/SM_Boat`.
2. Open `BP_Boat` (subclass of `BFLBoatPawn`).
3. Set `Hull` → Static Mesh = `SM_Boat`. Hide or delete the cabin cube.
4. Move `Rod` / `RodTip` to the real rod tip. Casts spawn from `RodTip`.

Until then the C++ boat is a blue box, a white cabin, and a grey cylinder rod.

### Trees and rocks

- Place a few `SM_Tree` / `SM_Rock` around the shore for scale.
- Or leave the auto-spawned primitives (`TreeCount` / `RockCount` on the GameMode).
- To disable them: Blueprint GameMode → `Auto Build Lake` = false, and build the level by hand.

### Atmosphere

- Auto-build already adds a sun tagged `BFL_Sun` and a sky light.
- `ABFLDayNightActor` rotates that sun. Disable with **Project Settings → Enable Day Night**.
- For a finished look, add a **Sky Atmosphere**, **Exponential Height Fog**, and an HDRI backdrop.

---

## Water height contract

Every system reads height from `UBFLStatics::GetWaterHeight`:

- Boat hull sits at `WaterHeight + 22`
- Bobber sits at `WaterHeight + 6` + sine
- Lure “hits water” when its Z crosses `WaterHeight`
- Fish wander at `WaterHeight - 40` to `-130`

If you move the lake up to Z = 500, set **Water Height = 500** or the boat will hover / sink.

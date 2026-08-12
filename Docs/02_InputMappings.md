# 02 — Input mappings

The boat builds a **runtime Enhanced Input** mapping if you do not assign one. Play works with zero Content assets.

`Config/DefaultInput.ini` already switches the project to Enhanced Input:

```ini
DefaultPlayerInputClass=/Script/EnhancedInput.EnhancedPlayerInput
DefaultInputComponentClass=/Script/EnhancedInput.EnhancedInputComponent
```

---

## Default bindings (runtime)

Created in `ABFLBoatPawn::BuildDefaultMappings`:

| Action | Keys | Value |
|---|---|---|
| Move | W/A/S/D, Gamepad Left X/Y | Axis2D |
| Look | Mouse2D, Gamepad Right X/Y | Axis2D |
| Cast / Reel | Left Mouse, Gamepad Right Trigger | Boolean (hold) |
| Cancel | Right Mouse, Gamepad Left Trigger, F | Boolean |

Cast is **hold to charge, release to throw**. The same button **holds to reel** during a fight.

---

## Option A — keep the runtime map (beginners)

Do nothing. If you need to change a key, edit `BuildDefaultMappings` in `Player/BFLBoatPawn.cpp`.

Example — also bind Space to cancel:

```cpp
IMC->MapKey(CancelAction, EKeys::SpaceBar);
```

---

## Option B — author Input assets in the editor (recommended later)

This is the “real” Unreal workflow and is what you want before shipping.

### 1. Create Input Actions

Content Browser → `/Game/Blueprints` (or a new `/Game/Input` folder) → **Input → Input Action**:

| Asset | Value Type |
|---|---|
| `IA_Move` | Axis2D (X = steer, Y = throttle) |
| `IA_Look` | Axis2D |
| `IA_Cast` | Digital (bool) |
| `IA_Cancel` | Digital (bool) |

### 2. Create the Mapping Context

**Input → Input Mapping Context** → `IMC_Fishing`.

Add mappings:

**IA_Move**

- `W` → Modifier **Swizzle Input Axis Values** = YXZ
- `S` → Swizzle YXZ + **Negate**
- `D` — no modifier
- `A` → Negate
- `Gamepad Left X` — no modifier
- `Gamepad Left Y` → Swizzle YXZ

**IA_Look**

- `Mouse 2D`
- `Gamepad Right X`
- `Gamepad Right Y` → Swizzle YXZ + Negate

**IA_Cast**

- `Left Mouse Button`
- `Gamepad Right Trigger`

**IA_Cancel**

- `Right Mouse Button`
- `Gamepad Left Trigger`
- `F`

### 3. Assign them

1. Create **Blueprint Class** based on `BFLBoatPawn` → `/Game/Blueprints/BP_Boat`.
2. Open it. In **Class Defaults**:
   - `Mapping Context` = `IMC_Fishing`
3. Create **Blueprint Class** based on `BFLPlayerController` → `BP_FishingController`.
   - `Override Mapping Context` = `IMC_Fishing` (optional duplicate; the pawn is enough).
4. Create **Blueprint Class** based on `BFLGameMode` → `BP_FishingGameMode`.
   - Default Pawn Class = `BP_Boat`
   - Player Controller Class = `BP_FishingController`
   - HUD Class = `BFLHUD`
5. **World Settings** (and/or Project Settings → Maps & Modes) → GameMode = `BP_FishingGameMode`.

You do **not** have to re-bind in Blueprint graphs. `SetupPlayerInputComponent` in C++ already binds `Move` / `Look` / `Cast` / `Cancel` on whatever `UInputAction` objects the pawn owns. If you want the asset actions instead of the runtime ones, add `UPROPERTY` setters or set the actions on a Blueprint-exposed interface later.

Practical shortcut: leave C++ runtime input as-is until you need rebindable controls or an options screen. Then replace `EnsureRuntimeInput` with loaded assets.

---

## How the hold / release cast is wired (C++)

```cpp
EIC->BindAction(CastAction, ETriggerEvent::Started,   this, &ABFLBoatPawn::CastStarted);
EIC->BindAction(CastAction, ETriggerEvent::Completed, this, &ABFLBoatPawn::CastCompleted);
EIC->BindAction(CastAction, ETriggerEvent::Canceled,  this, &ABFLBoatPawn::CastCompleted);
```

`CastStarted` → `Fishing->StartCastCharge()`
`CastCompleted` → `Fishing->ReleaseCast()`

If the state is already `Fighting`, those same calls start / stop reeling.

---

## Gamepad notes

- Left stick dead zone is `0.25` in `DefaultInput.ini`.
- Right trigger is treated as a digital button (threshold). That matches “hold / release”.
- If look Y feels inverted, remove the `Negate` modifier on `Gamepad_RightY` in `BuildDefaultMappings`.

# 03 — Fishing system

All of this lives on `UBFLFishingComponent`, attached to the boat.

## Cast

1. Player holds Cast. State → `ChargingCast`. `CastPower` goes `0 → 1` over `ChargeTime` (1.35s).
2. A gold debug sphere / ring previews the splash point (`PredictLanding`).
3. On release, `ABFLCastProjectile` spawns at the rod tip.
4. Velocity = camera-forward (flattened) + loft, scaled between `MinCastSpeed` and `MaxCastSpeed`.
5. Each tick: `Velocity.Z += GravityZ * dt`. When the arc crosses `WaterHeight`, it is a hit.
6. If the hit is inside the lake radius → spawn `ABFLBaitActor`. State → `Waiting`.
7. If the hit is on the shore → notification, back to `Idle`.

The fishing line is a `UCableComponent` from `RodTip` to the lure, then to the bobber.

## Bait

`ABFLBaitActor`:

- Red sphere bobber + small hook mesh.
- Bobs on the water plane.
- Registers itself with `ABFLGameMode` as the **one** active bait.
- `AttractRadius` is queried by fish (they also use their own `DetectRadius`).
- Lifetime 45s, then it despawns if nobody bit.

## Bite

A fish calls `UBFLFishingComponent::NotifyFishBite(this)` after its own delay + random roll (see [04 Fish AI](04_FishAI.md)).

The component then:

- Rejects the bite unless state is `Waiting` and bait is still available.
- Stores `HookedFish`, sets tension to `0.22`, progress to `0`.
- State → `Fighting`.
- Broadcasts **FISH ON!** plus the species name.

## Tension meter and reel

Each fight tick (`TickFighting`):

```
FishPull     = species.FightIntensity * pulse(sine + slow surge)
Tension     += FishPull * 0.22 * dt
if reeling:  Tension += ReelTensionRate * dt
             if Tension > 0.70: extra OverreelPenalty
else:        Tension -= TensionDecayRate * dt
Tension      = clamp(floor, 1)

if reeling:  ReelProgress += (1.15 - 0.85*Tension) * dt / species.StaminaSeconds
else:        ReelProgress -= 0.06 * dt   // fish takes line back
```

**Success:** `ReelProgress >= 1` → `LandFish()`
**Failure:** `Tension >= 1` → `SnapLine()`

`LandFish`:

- Reads species + weight from the fish.
- `GameMode->RegisterCatch` (score += `weight * ScorePerPound`).
- Destroys the fish.
- Shows **Fish caught!** with name, weight, points.

`SnapLine`:

- Releases the fish (`Flee`).
- Tells every fish to flee for 5 seconds (the lake “spooks”).
- Shows **Line snapped!**

## Key functions (call these from Blueprint)

| Function | When |
|---|---|
| `StartCastCharge` | Cast pressed |
| `ReleaseCast` | Cast released |
| `StartReeling` / `StopReeling` | Fight hold / release |
| `Cancel` | Cut line / abort charge |
| `NotifyCastHitWater` | Lure hit (already called by the projectile) |
| `NotifyFishBite` | Fish committed (already called by the AI) |

## Delegates to bind in Blueprint

- `OnStateChanged(EFishingState)`
- `OnCastPowerChanged(float)`
- `OnTensionChanged(float Tension, float ReelProgress)`
- `OnFishCaught(EFishSpecies, float WeightLbs)`
- `OnLineSnapped`

## Creating the bait actor yourself

You do not have to — C++ spawns `ABFLBaitActor`. To restyle it:

1. **Blueprint Class** based on `BFLBaitActor` → `/Game/Blueprints/BP_Bait`.
2. Replace `Bobber` / `Hook` static meshes.
3. On `BP_Boat` (or the C++ boat defaults) set `Fishing → Bait Class` = `BP_Bait`.
4. Same pattern for `Projectile Class` if you want a lure mesh / Niagara splash.

If you want a pure-Blueprint bait:

1. New Actor, add a Sphere collision (root), a Static Mesh, tick Enabled.
2. **Event BeginPlay** → get `BFLGameMode` → `Register Bait`.
3. **Event EndPlay** → `Unregister Bait`.
4. **Event Tick** → set Z = `Get Water Height` + sin bob.
5. Set that class on the fishing component.

# 04 — Fish AI

Class: `ABFLFishActor`. No Behavior Tree, no NavMesh. A tick state machine so it stays readable.

## Creating a fish

**C++ path (already done):** `ABFLGameMode::SpawnStartingFish` picks a weighted species and calls `SpawnFish`.

**Editor path:**

1. **Blueprint Class** based on `BFLFishActor` → `/Game/Blueprints/BP_Fish`.
2. Swap `Body` / `Tail` meshes for a real bass.
3. Place a few in the lake **below** the water plane (Z = `-40` to `-130`).
4. Or set `BP_FishingGameMode` to spawn `BP_Fish` by changing `SpawnFish` in a Blueprint override.

The GameMode only auto-spawns enough to reach `FishCount`. Placed fish count toward that total.

## States

| State | Behavior |
|---|---|
| `Wander` | Swim toward a random point in the lake. Every 8s or on arrival, pick a new point. |
| `Approaching` | Bait is inside `DetectRadius`. Swim at 1.25× speed toward the bobber. |
| `DecidingBite` | Inside `BiteRadius`. Hover under the bobber for `0.45–1.8s`. |
| `Fighting` | Circle-tug. Moves the bobber with it. |
| `Flee` | Sprint toward the far side of the lake, then wander. Ignores bait. |

## Bite logic (the important roll)

```
Event: DecidingBite timer finished
  if bait is gone or already hooked:
      wander + 2s cooldown
  else if FRand() > BiteChance:
      wander + 4s cooldown          // inspected, refused
  else:
      FishingComponent.NotifyFishBite(this)
      state = Fighting
```

`BiteChance` is per species (crappie 0.75, trophy 0.28).

Only one fish can hook the bait (`ABFLBaitActor::IsAvailable`).

## Detect bait

Every wander tick:

```
Bait = GameMode.GetActiveBait()
if Bait and distance(self, Bait) <= DetectRadius:
    state = Approaching
```

No overlap volumes required. With ~12 fish this is cheap.

## Flee

Called when:

- The player snaps the line (`GameMode.NotifyAllFishFlee(5)`).
- This fish loses the fight (`OnLineReleased(false)`).
- The bait disappears mid-fight.

## Species

`UBFLFishCatalog` ships four:

| Species | Weight | Bite | Fight | Stamina | Spawn weight |
|---|---|---|---|---|---|
| Largemouth Bass | 1.5–8 lb | 0.55 | medium | 6s | 1.00 |
| Smallmouth Bass | 1–5.5 lb | 0.50 | high | 5.5s | 0.80 |
| Crappie | 0.4–2.2 lb | 0.75 | low | 3.2s | 0.90 |
| Trophy Largemouth | 8–14.5 lb | 0.28 | very high | 11s | 0.15 |

To add a species without C++:

1. Content Browser → **Miscellaneous → Data Asset** → `BFLFishCatalog` → `/Game/Data/DA_FishCatalog`.
2. Duplicate a row, change color, weights, chances.
3. Assign `DA_FishCatalog` on `BP_FishingGameMode` → `Fish Catalog`.

To add a *new enum value* you still need one line in `EFishSpecies` (`BFLTypes.h`) and a `MakeBuiltin` case.

## Blueprint graph (equivalent brain)

If you rebuild this in Blueprint on a pawn:

```
Event Tick
  → Sequence
      → Wag tail (sin on yaw)
      → Switch on AIState
           Wander:       MoveTo(WanderTarget) → if arrived, PickRandomPointInLake
                         DistanceTo(ActiveBait) < DetectRadius? → SetState Approaching
           Approaching:  MoveTo(Bait)
                         Distance < BiteRadius? → SetState DecidingBite, BiteDelay = Random(0.45, 1.8)
           DecidingBite: MoveTo(Bait - 28z)
                         StateTime >= BiteDelay?
                           Branch (RandomFloat < BiteChance AND Bait.IsAvailable)
                             True  → Get Owner of Bait → Get Component Fishing → Notify Fish Bite
                             False → SetState Wander
           Fighting:     Orbit FightCenter, SetActorLocation(Bait) to my XY
           Flee:         MoveTo(WanderTarget) at 1.8× speed → then Wander
```

`PickRandomPointInLake` is already a Blueprint function on `BFLStatics`.

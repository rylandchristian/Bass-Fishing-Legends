# 07 — Blueprint guide

You do **not** need these graphs to play. They are the Blueprint equivalents of the C++ so you can reimplement or extend visually.

---

## BP_Boat — movement (already in C++)

If you ever rebuild movement in a Blueprint pawn:

```
Event Tick
  → Add Actor World Rotation (Yaw = MoveX * TurnSpeed * Delta)
  → Get Actor Forward Vector * (MoveY * MaxSpeed * Delta)
  → Add Actor World Offset
  → Set Actor Location Z = Get Water Height + 22
```

Input (Enhanced Input, same actions as [02](02_InputMappings.md)):

```
IA_Move  Triggered  → Set MoveX = Action Value X
                       Set MoveY = Action Value Y
IA_Look  Triggered  → Add SpringArm yaw / pitch
IA_Cast  Started    → Fishing.Start Cast Charge
IA_Cast  Completed  → Fishing.Release Cast
IA_Cancel Started   → Fishing.Cancel
```

---

## BP_CastProjectile — water hit

```
Event Tick
  → Velocity.Z += GravityZ * Delta
  → NewLoc = Location + Velocity * Delta
  → Branch (OldZ > WaterHeight AND NewZ <= WaterHeight)
       True  → Hit = Lerp(Old, New, (OldZ-WaterHeight)/(OldZ-NewZ))
               Get Owner → Get Component Fishing → Notify Cast Hit Water (Hit)
               Destroy Actor
       False → Set Actor Location (NewLoc)
```

---

## BP_Bait

```
Event BeginPlay
  → Get BFL Game Mode → Register Bait (self)

Event EndPlay
  → Get BFL Game Mode → Unregister Bait (self)

Event Tick
  → Z = Get Water Height + 6 + sin(Time * 2.2) * 4
  → Set Actor Location
```

---

## Bite (on BP_Fish)

```
Custom Event CommitBite
  → Branch (Bait.Is Available)
       False → Set AI State Wander
       True  → Branch (Random Float in Range(0,1) <= Bite Chance)
            False → Set AI State Wander
            True  → Bait.Get Owner
                    → Get Component by Class (BFLFishingComponent)
                    → Notify Fish Bite (self)
```

---

## Tension meter (on a Widget or the boat)

C++ already ticks this. Blueprint version of the fight step:

```
Event Tick (only when State == Fighting)
  → Pulse = 0.5 + 0.5 * sin(Time)
  → Tension += FightIntensity * 0.22 * Pulse * Delta
  → Branch (Is Reeling)
       True  → Tension += 0.28 * Delta
               ReelProgress += (1.15 - 0.85 * Tension) * Delta / Stamina
       False → Tension -= 0.22 * Delta
  → Clamp Tension 0.08..1
  → Clamp ReelProgress 0..1
  → Branch Tension >= 1 → Snap (broadcast, flee fish, State = Failed)
  → Branch ReelProgress >= 1 → Catch (Register Catch, destroy fish, State = Caught)
```

Wire a Progress Bar percent to `Tension` and another to `ReelProgress`.

---

## BP_GameMode catch

```
Custom Event RegisterCatch (Species, Weight)
  → Def = Get Species Def (Species)
  → Points = Round(Weight * Def.Score Per Pound)
  → Score += Points
  → Fish Caught += 1
  → Add to Catch Log
  → Broadcast On Fish Caught
  → Broadcast Notification "Fish caught! {Name} {Weight} lb +{Points}"
```

---

## Suggested Blueprint subclasses (do these when you start art)

| Parent | Blueprint | Why |
|---|---|---|
| `BFLGameMode` | `BP_FishingGameMode` | Assign catalog, pawn, HUD; toggle auto-lake |
| `BFLBoatPawn` | `BP_Boat` | Real hull mesh, rod socket |
| `BFLFishActor` | `BP_Fish` | Real bass mesh / animation |
| `BFLBaitActor` | `BP_Bait` | Bobber mesh, splash Niagara |
| `BFLCastProjectile` | `BP_Lure` | Lure mesh, trail |
| `BFLHUD` | `BP_HUD` | Or replace entirely with UMG |

Always change the GameMode defaults — do not edit engine-default CDOs.

# Bass Fishing Legends

A lightweight, beginner-friendly bass-fishing game for **Unreal Engine 5.8.1**. You drive a small boat, charge-cast a line, wait for a bite, then manage a tension meter to land the fish.

The game is written in **C++** so it is playable after you compile — no Blueprint assets are required to hit Play. Every major class is `Blueprintable` / `BlueprintCallable` so you can extend it in the editor later.

> **Engine location:** the Linux editor lives at `/home/wade/UnrealEngine/UE_5.8` (moved off the Desktop so it is not sitting inside this git repo). The `.uproject` is associated with **5.8**.

---

## How to play

| Input | Action |
|---|---|
| **WASD** / left stick | Drive the boat |
| **Mouse** / right stick | Look (cast aims where you look) |
| **Hold LMB** / **Right Trigger** | Charge cast. A gold ring shows the landing spot |
| **Release LMB** / **RT** | Throw the lure |
| **Hold LMB** / **RT** again (while hooked) | Reel in |
| **Release** during the fight | Let tension drop |
| **RMB** / **Left Trigger** / **F** | Cancel the cast or cut the line |

**Tension:** green is safe, yellow is risky, red snaps the line. Reel while the bar is in green/yellow. Ease off when it spikes.

**Score:** `weight (lb) × species multiplier`. Trophy bass are rare and fight hard.

---

## 15-minute first run

The engine is already on this machine at `/home/wade/UnrealEngine/UE_5.8`.

```bash
# compile the game module (first time)
./scripts/build-editor.sh

# open the editor
./scripts/launch-editor.sh
```

Or pass the `.uproject` straight to the editor:

```bash
/home/wade/UnrealEngine/UE_5.8/Engine/Binaries/Linux/UnrealEditor \
  /home/wade/dev/games/Bass-Fishing-Legends/BassFishingLegends.uproject
```

When the editor opens, press **Play** (Alt+P). The GameMode builds a lake, boat, fish, and lighting on the blank template map. Drive out, hold left mouse to charge, release to cast, wait for **FISH ON!**, then reel.

That is enough to confirm the loop. Then follow the docs to make a real map and swap placeholders.

---

## Folder structure

```
Bass-Fishing-Legends/
├── BassFishingLegends.uproject
├── Config/                         # Engine, input, game settings
├── Content/                        # Create assets here in the editor
│   ├── Audio/
│   ├── Blueprints/                 # BP_Boat, BP_Fish, BP_Bait, ...
│   ├── Characters/
│   ├── Data/                       # DA_FishCatalog
│   ├── Maps/                       # Lake.umap
│   ├── Materials/                  # M_Water, M_Hull, ...
│   ├── Meshes/                     # Boat, fish, rocks (replace primitives)
│   └── UI/                         # Optional UMG widgets
├── Docs/                           # Step-by-step guides
└── Source/BassFishingLegends/
    ├── BFLTypes.h                  # Enums, catch record, delegates
    ├── Game/                       # GameMode, settings, helpers, controller
    ├── Player/                     # Boat pawn + runtime Enhanced Input
    ├── Fishing/                    # Cast, bait, tension / reel
    ├── Fish/                       # Species catalog + wander / bite AI
    ├── UI/                         # Tension meter, score, notifications
    └── World/                      # Day / night sun
```

---

## Systems (what lives where)

| System | Class | Role |
|---|---|---|
| Session / score | `ABFLGameMode` | Score, catch log, fish roster, lake bootstrap |
| Tuning | `UBFLGameSettings` | Project Settings → Game → Bass Fishing Legends |
| Boat | `ABFLBoatPawn` | Drive, look, rod tip, fishing line cable |
| Cast / fight | `UBFLFishingComponent` | Charge, projectile, bait, tension, reel |
| Lure | `ABFLCastProjectile` | Ballistic arc, water-plane hit |
| Bobber | `ABFLBaitActor` | Attracts fish, hooks one |
| Fish | `ABFLFishActor` | Wander → approach → bite roll → fight / flee |
| Species | `UBFLFishCatalog` | Largemouth, smallmouth, crappie, trophy |
| HUD | `ABFLHUD` | Tension, reel, score, “Fish caught!” |
| Time | `ABFLDayNightActor` | Rotates the sun over a 24h clock |

Fishing state machine:

```
Idle → ChargingCast → Casting → Waiting → Fighting → Caught → Idle
                                      ↘           ↘ Failed → Idle
                                       cancel
```

Fish AI state machine:

```
Wander → Approaching → DecidingBite → Fighting
                ↘ miss / lose interest ↗
Any → Flee → Wander
```

---

## Docs

| Guide | Contents |
|---|---|
| [01 Project setup](Docs/01_ProjectSetup.md) | Install engine, generate project files, first compile |
| [02 Input mappings](Docs/02_InputMappings.md) | Runtime defaults + how to author IMC / IA assets |
| [03 Fishing system](Docs/03_FishingSystem.md) | Cast, bait, bite, tension, reel |
| [04 Fish AI](Docs/04_FishAI.md) | Wander, detect, bite chance, flee |
| [05 UI](Docs/05_UI.md) | HUD now, UMG later |
| [06 Environment](Docs/06_Environment.md) | Lake map, water, trees, rocks |
| [07 Blueprint guide](Docs/07_BlueprintGuide.md) | Node-by-node graphs if you prefer Blueprints |
| [08 Checklist](Docs/08_Checklist.md) | “Does it run?” sign-off |
| [09 Stretch goals](Docs/09_StretchGoals.md) | Species, day/night, weight scoring — already stubbed |

---

## Expanding later

- **New species:** add an enum value in `BFLTypes.h` *or* create a `UBFLFishCatalog` Data Asset in `/Game/Data` and assign it on the GameMode.
- **Real meshes:** drop a boat / bass into `/Game/Meshes` and set them on a Blueprint subclass of `ABFLBoatPawn` / `ABFLFishActor`.
- **UMG HUD:** keep `ABFLHUD` as a fallback, or replace `HUDClass` on a Blueprint GameMode. Bind to `UBFLFishingComponent` delegates.
- **Water plugin:** enable Water + Landmass and place a `WaterBodyLake`. Tag it `WaterSurface` so auto-build does not spawn the placeholder plane.
- **Sounds:** the loop already has hook points (state changes, catch, snap). Add `USoundBase` cues in a Blueprint subclass.

---

## License

Starter project. Use and modify freely.

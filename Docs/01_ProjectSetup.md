# 01 — Project setup

## Engine version

This machine uses the **Linux Unreal Engine 5.8.1** installed build.

| Item | Path |
|---|---|
| Engine root | `/home/wade/UnrealEngine/UE_5.8` |
| Editor | `/home/wade/UnrealEngine/UE_5.8/Engine/Binaries/Linux/UnrealEditor` |
| Registered as | `UE_5.8` in `~/.config/Epic/UnrealEngine/Install.ini` |
| Project association | `"EngineAssociation": "5.8"` |

The 24 GB engine was moved **off the Desktop** and **out of the git repo**. Unreal does not live inside the game folder — the project only points at it.

---

## Launch on this machine

```bash
cd /home/wade/dev/games/Bass-Fishing-Legends
./scripts/build-editor.sh      # first time, or after C++ changes
./scripts/launch-editor.sh     # open the editor
```

Manual equivalent:

```bash
ENGINE=/home/wade/UnrealEngine/UE_5.8
PROJECT=/home/wade/dev/games/Bass-Fishing-Legends/BassFishingLegends.uproject

"$ENGINE/Engine/Build/BatchFiles/Linux/GenerateProjectFiles.sh" -project="$PROJECT" -game

"$ENGINE/Engine/Build/BatchFiles/Linux/Build.sh" \
  BassFishingLegendsEditor Linux Development -Project="$PROJECT" -WaitMutex

"$ENGINE/Engine/Binaries/Linux/UnrealEditor" "$PROJECT"
```

First compile takes several minutes. Watch the Output Log for `LogBFL: Bass Fishing Legends ready`.

---

## Windows 11 — rebuild after a compile failure

Close the Unreal Editor first. In **PowerShell** or **cmd**, from the project folder:

```bat
git pull origin dev
scripts\rebuild-editor.bat
```

If your engine is not at `C:\UE_5.8`:

```bat
set UE_ROOT=C:\Path\To\UE_5.8
scripts\rebuild-editor.bat
```

The script deletes `Intermediate` and `Binaries`, then compiles `BassFishingLegendsEditor`. If it fails, open `%LOCALAPPDATA%\UnrealBuildTool\Log.txt` and search for `error C`.

Do **not** click Play until that script prints `Build succeeded`. Then double-click `BassFishingLegends.uproject`.

The folder name can contain spaces (`Bass-Fishing-Legends 5.8 5.8`). That is fine as long as you run the script from *that* clone after `git pull`.

---

## If the engine is elsewhere

Set `UE_ROOT` before the scripts, or edit `~/.config/Epic/UnrealEngine/Install.ini`:

```ini
[Installations]
UE_5.8=/absolute/path/to/engine
```

Then:

```bash
UE_ROOT=/absolute/path/to/engine ./scripts/launch-editor.sh
```

---

## What should already be wired

| Setting | Value | File |
|---|---|---|
| Default GameMode | `BFLGameMode` | `Config/DefaultEngine.ini` |
| Default map | Engine blank template | same |
| Enhanced Input player + component | enabled | `Config/DefaultInput.ini` |
| Plugins | EnhancedInput, CableComponent | `.uproject` |
| Tunables | water height, fish count, boat speed | `Config/DefaultGame.ini` |

On **Play**, `ABFLGameMode::StartPlay` will:

1. Spawn a water plane, shore, trees, rocks, a small island.
2. Spawn a PlayerStart and a directional + sky light if missing.
3. Spawn `ABFLDayNightActor`.
4. Spawn 12 fish (weighted species mix).
5. Possess `ABFLBoatPawn`.

You do **not** need to place anything to get a playable session.

---

## Recommended editor folders

In the Content Browser, these folders already exist on disk. They will appear after you refresh or add one asset:

| Folder | Put here |
|---|---|
| `/Game/Blueprints` | `BP_Boat`, `BP_Fish`, `BP_Bait`, `BP_GameMode` |
| `/Game/Maps` | `Lake` |
| `/Game/Meshes` | Boat, bass, rocks, trees |
| `/Game/Materials` | Water, hull, fish skin |
| `/Game/UI` | UMG tension widget (optional) |
| `/Game/Audio` | Cast whoosh, splash, reel, splash-catch |
| `/Game/Data` | `DA_FishCatalog` |
| `/Game/Characters` | Future angler mesh |

---

## Create your own map (optional, 3 minutes)

1. **File → New Level → Empty Level** (or Basic).
2. **File → Save Current Level As…** → `/Game/Maps/Lake`.
3. **Window → World Settings**. GameMode Override → `BFLGameMode` (or a Blueprint subclass).
4. **Edit → Project Settings → Maps & Modes**
   - Editor Startup Map = `Lake`
   - Game Default Map = `Lake`
5. Place a **Player Start** at `(0, 0, 40)` if you want a specific spawn. Otherwise the GameMode creates one.
6. Either leave `bAutoBuildLake` on (placeholder lake appears at Play) **or** build your own water and tag the surface actor `WaterSurface` so auto-build skips.

---

## Project Settings to glance at

**Edit → Project Settings → Game → Bass Fishing Legends**

- `Water Height` — Z of the water plane (default `0`)
- `Lake Radius` — playable circle (default `4500`)
- `Fish Count` — how many spawn (default `12`)
- `Min / Max Cast Speed` — charge-cast range
- `Boat Max Speed`
- `Enable Day Night` / `Day Length Minutes`

These write to `Config/DefaultGame.ini`.

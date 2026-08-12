# 09 — Stretch goals

These were optional. The project already ships a working stub for each one.

## Different fish species — done

Four species in `UBFLFishCatalog`:

- Largemouth Bass (default)
- Smallmouth Bass (faster, meaner fight)
- Crappie (easy meal)
- Trophy Largemouth (rare, 8–14.5 lb, long fight)

Add more with a Data Asset (`/Game/Data/DA_FishCatalog`) assigned on the GameMode. See [04 Fish AI](04_FishAI.md).

**Next step:** a unique mesh / material per species on `ABFLFishActor::ApplyVisuals`, or a `TMap<EFishSpecies, UStaticMesh*>` on a Blueprint subclass.

## Day / night cycle — done

`ABFLDayNightActor` rotates the sun tagged `BFL_Sun`, shifts color from dawn orange → noon white → night blue, and dims intensity at night.

- Default: 12 real minutes = 24 game hours, starting ~09:30.
- Toggle: **Project Settings → Bass Fishing Legends → Enable Day Night**.
- Call `Set Time Of Day` from Blueprint (e.g. a debug key).

**Next step:** drive a Sky Atmosphere / Exponential Height Fog from the same hours value, and change fish `DetectRadius` / `BiteChance` at dusk.

## Inventory / weight scoring — done

- Each catch stores `Species`, `DisplayName`, `WeightLbs`, `ScoreAwarded`, `TimeSeconds` in `ABFLGameMode::CatchLog`.
- Score = `round(Weight * ScorePerPound)` (trophy is 25 pts/lb).
- HUD lists the last four fish.

**Next step:** a UMG inventory (`WBP_Livewell`) bound to `GetCatchLog`, plus a “keep / release” prompt on `Caught` that only calls `RegisterCatch` if the player keeps it.

## Other easy expansions

| Idea | Hook |
|---|---|
| Lure types | Subclass `ABFLBaitActor`, change `AttractRadius` and a `PreferredSpecies` filter the AI checks before approaching |
| Depth / structure | Give fish a home waypoint (weed line, island edge) instead of a full-lake wander |
| Livewell limit | Cap `CatchLog` at 5; extra fish must be released |
| Simple tournament | `ABFLGameMode` timer + “best 3 fish by weight” |
| Splash FX | Niagara at `NotifyCastHitWater` and on `LandFish` |
| Audio | Play 2D sound in `OnStateChanged` / `OnFishCaught` / `OnLineSnapped` |
| Better water | Water plugin body tagged `WaterSurface` |

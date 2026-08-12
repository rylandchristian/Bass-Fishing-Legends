# 08 — Final checklist

Work top to bottom. A fresh clone should pass the **Must work** section after one compile.

## Must work (C++ only)

- [ ] Unreal Engine **5.8.1** is at `/home/wade/UnrealEngine/UE_5.8`.
- [ ] `BassFishingLegends.uproject` opens and the `BassFishingLegends` module compiles with **0 errors**.
- [ ] **Play** from the default template map.
- [ ] Output Log contains `LogBFL: Bass Fishing Legends ready`.
- [ ] You spawn in a blue boat on a teal lake with trees / rocks around the rim.
- [ ] **WASD** moves the boat. It stays on the water and cannot leave the circle.
- [ ] Mouse orbits the camera.
- [ ] Hold **left mouse**: a gold cast-power bar appears, a gold ring shows the landing.
- [ ] Release: a yellow lure flies in an arc. A red bobber appears on the water. A cable hangs from the rod.
- [ ] After a few seconds a fish swims over. Either it bites or it peels off (that is the random roll).
- [ ] On a bite: **FISH ON!**, tension bar + reel bar appear.
- [ ] Hold left mouse to reel. Release when the bar goes red.
- [ ] Landing a fish: **Fish caught!** with name, weight, points. Score increments. Catch log updates. Fish disappears.
- [ ] Snapping: **LINE SNAPPED**, nearby fish scatter, you can cast again.
- [ ] **F** / right mouse cancels a waiting bobber.
- [ ] Gamepad left stick drives, right trigger charges/reels (if a pad is plugged in).

## Project settings

- [ ] Maps & Modes → Default GameMode = `BFLGameMode` (already in `DefaultEngine.ini`).
- [ ] Input → default player input / component classes are Enhanced Input (already in `DefaultInput.ini`).
- [ ] Plugins: **Enhanced Input** and **Cable Component** enabled (already in the `.uproject`).
- [ ] **Edit → Project Settings → Game → Bass Fishing Legends** values look sane.

## Optional art pass

- [ ] `/Game/Maps/Lake` saved and set as Game Default Map + Editor Startup Map.
- [ ] Water surface tagged `WaterSurface` and `Water Height` matches its Z.
- [ ] `BP_Boat` uses a real hull; `RodTip` sits at the rod end.
- [ ] `BP_Fish` uses a bass mesh; at least one of each species placed or spawned.
- [ ] `M_Water` applied (or Water Body Lake).
- [ ] A few authored trees / rocks on the bank.
- [ ] UMG `WBP_FishingHUD` added (Canvas HUD hidden or removed).

## Common failures

| Symptom | Fix |
|---|---|
| Module missing / compile prompt loops | Build `BassFishingLegendsEditor` from Visual Studio / `Build.sh`. Read the first C++ error. |
| Play, but no boat | World Settings GameMode is not `BFLGameMode`. |
| Boat flies / sinks | `Water Height` does not match the plane. |
| Cast never lands | Camera aimed at the sky; loft still hits eventually. Look toward the water. |
| Fish ignore bait | Bait is outside `Lake Radius`, or all fish are in `Flee` after a snap (wait 5s). |
| No tension bar | Bite never happened. Watch for fish gathering under the bobber. Crappie bite more often. |
| Keys do nothing | Enhanced Input not set in `DefaultInput.ini`, or another IMC consumed the keys. |
| Cable missing | Cable Component plugin disabled. |
| “Create Widget” HUD is empty | You hid `BFLHUD` and forgot to `Add to Viewport` a UMG widget. |

## Package test

- [ ] **Platforms → Windows → Package Project** (or Linux).
- [ ] Cooked build launches to the lake and the cast → bite → reel loop still works.
- [ ] `/Game/Maps/Lake` is in **Packaging → List of maps to include** if you created one.

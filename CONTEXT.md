# Bass Fishing Legends

A personal bass-fishing toy: one lake, a boat, and a single fishing sequence. It may become a larger project later if that sequence feels good.

## Language

**Fishing**:
The player sequence from idle through a land or a snap: charge-cast, wait, fight, then land or snap.
_Avoid_: core loop, gameplay loop, run (when we mean this sequence)

**Charge-cast**:
Holding to set throw power, then releasing to send the lure.
_Avoid_: throw (when we mean this charged action)

**Wait**:
The lure is in the water and a fish has not bitten yet.
_Avoid_: idle (idle is before a charge-cast)

**Fight**:
A fish is on the line. The player reels against tension. It ends in a land or a snap.
_Avoid_: hooked (when we mean the whole contest)

**Tension**:
How close the line is to a snap, from slack to breaking.
_Avoid_: stress, pressure, meter (when we mean this value)

**Reel**:
How close the fight is to a land. Reeling raises it. Easing off lets a slow leak — the fish takes a little back.
_Avoid_: progress, catch meter (when we mean this value)

**Pump**:
Reel while tension is not red. Ease off in red. Repeat until a land.
_Avoid_: mash, hold (when we mean this skill)

**Land**:
The player wins the fight. Today a land always becomes a catch.
_Avoid_: catch (when we mean the moment, not the record)

**Catch**:
A landed fish recorded for score: species, weight, points, and time.
_Avoid_: landing, keep (keep is not a concept yet)

**Snap**:
The line breaks during a fight. The fish is not a catch.
_Avoid_: fail, break (when we mean this outcome)

**Score**:
Points from a catch: weight times that species' points per pound.
_Avoid_: XP, rating

## World

**Boat**:
The small jon boat the player drives and possesses.
_Avoid_: pawn (when we mean the vessel), vehicle, bass boat (when we mean this vessel)

**Angler**:
The visible person on the boat. The player does not possess the Angler.
_Avoid_: person, character, player, avatar

**Rod**:
The fishing rod the Angler holds. A charge-cast starts from its tip.
_Avoid_: stick, pole (when we mean this object)

**Lake**:
The single playable body of water and the shore around it.
_Avoid_: map, level, world (when we mean this place)

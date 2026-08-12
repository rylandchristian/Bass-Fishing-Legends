#!/usr/bin/env bash
# Launch Bass Fishing Legends in the local Unreal Engine 5.8.1 editor.
set -euo pipefail

# The 5.8.1 Linux installed build ships a stub DotNet host with no host/fxr.
export UE_USE_SYSTEM_DOTNET=1
# Editor rpath does not include the FBX third-party folder.
export LD_LIBRARY_PATH="/home/wade/UnrealEngine/UE_5.8/Engine/Binaries/Linux:${LD_LIBRARY_PATH:-}"

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ENGINE="${UE_ROOT:-/home/wade/UnrealEngine/UE_5.8}"
EDITOR="$ENGINE/Engine/Binaries/Linux/UnrealEditor"
PROJECT="$ROOT/BassFishingLegends.uproject"

if [[ ! -x "$EDITOR" ]]; then
	echo "Unreal Editor not found at: $EDITOR" >&2
	echo "Expected the engine at /home/wade/UnrealEngine/UE_5.8" >&2
	exit 1
fi

exec "$EDITOR" "$PROJECT" "$@"

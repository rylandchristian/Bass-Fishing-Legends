#!/usr/bin/env bash
# Compile the BassFishingLegends editor module (Linux).
# Override the engine with:  UE_ROOT=/path/to/UE_5.8 ./scripts/build-editor.sh
set -euo pipefail

export UE_USE_SYSTEM_DOTNET=1

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PROJECT="$ROOT/BassFishingLegends.uproject"

resolve_engine() {
	local candidate
	for candidate in \
		"${UE_ROOT:-}" \
		"${HOME}/UnrealEngine/UE_5.8" \
		"/home/wade/UnrealEngine/UE_5.8"
	do
		if [[ -n "$candidate" && -x "$candidate/Engine/Build/BatchFiles/Linux/Build.sh" ]]; then
			printf '%s\n' "$candidate"
			return 0
		fi
	done
	return 1
}

ENGINE="$(resolve_engine)" || {
	echo "Build.sh not found. Set UE_ROOT to your UE 5.8 engine root." >&2
	exit 1
}

exec "$ENGINE/Engine/Build/BatchFiles/Linux/Build.sh" \
	BassFishingLegendsEditor Linux Development \
	-Project="$PROJECT" \
	-WaitMutex \
	-FromMsBuild

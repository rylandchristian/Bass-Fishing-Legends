#!/usr/bin/env bash
# Compile the BassFishingLegends editor module against the local UE 5.8.1 install.
set -euo pipefail

# The 5.8.1 Linux installed build ships a stub DotNet host with no host/fxr.
export UE_USE_SYSTEM_DOTNET=1

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ENGINE="${UE_ROOT:-/home/wade/UnrealEngine/UE_5.8}"
PROJECT="$ROOT/BassFishingLegends.uproject"
BUILD="$ENGINE/Engine/Build/BatchFiles/Linux/Build.sh"

if [[ ! -x "$BUILD" ]]; then
	echo "Build.sh not found at: $BUILD" >&2
	exit 1
fi

exec "$BUILD" BassFishingLegendsEditor Linux Development \
	-Project="$PROJECT" \
	-WaitMutex \
	-FromMsBuild

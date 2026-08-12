#!/usr/bin/env bash
# Launch Bass Fishing Legends in Unreal Editor (Linux).
# Override the engine with:  UE_ROOT=/path/to/UE_5.8 ./scripts/launch-editor.sh
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
		if [[ -n "$candidate" && -x "$candidate/Engine/Binaries/Linux/UnrealEditor" ]]; then
			printf '%s\n' "$candidate"
			return 0
		fi
	done
	return 1
}

ENGINE="$(resolve_engine)" || {
	echo "Unreal Editor not found. Set UE_ROOT to your UE 5.8 engine root." >&2
	echo "Example: UE_ROOT=\"\$HOME/UnrealEngine/UE_5.8\" $0" >&2
	exit 1
}

EDITOR="$ENGINE/Engine/Binaries/Linux/UnrealEditor"
export LD_LIBRARY_PATH="$ENGINE/Engine/Binaries/Linux:${LD_LIBRARY_PATH:-}"

# VMware's GPU does not pass UE 5.8's Vulkan SM5 profile. Use Mesa lavapipe
# (software Vulkan) so a window can come up on that host. Real GPUs skip this.
if [[ -z "${VK_ICD_FILENAMES:-}" && -f /usr/share/vulkan/icd.d/lvp_icd.json ]]; then
	if grep -q -i vmware /proc/cpuinfo 2>/dev/null || [[ -e /sys/class/dmi/id/sys_vendor && "$(cat /sys/class/dmi/id/sys_vendor 2>/dev/null)" == *VMware* ]]; then
		export VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/lvp_icd.json
	fi
fi

exec "$EDITOR" "$PROJECT" \
	-DisablePlugins=PythonScriptPlugin,PlatformCrypto \
	"$@"

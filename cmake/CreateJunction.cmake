# This script creates a junction (symbolic link) on Windows using PowerShell.
# Called at build time by the DodoResources target via cmake -P.
# Required variables (passed with -D):
#   LINK_DIR   - path where the junction will be created
#   TARGET_DIR - path the junction points to
#
# The script is idempotent: an existing link that already points at TARGET_DIR is left alone.
# A stale link is unlinked without recursing, so the contents of its target are never touched.

set(_SCRIPT [=[
$ErrorActionPreference = 'Stop'
$link = [IO.Path]::GetFullPath($env:DD_LINK_DIR)
$target = [IO.Path]::GetFullPath($env:DD_TARGET_DIR)
$item = Get-Item -LiteralPath $link -Force -ErrorAction SilentlyContinue
if ($item) {
    if ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) {
        if (@($item.Target)[0] -eq $target) { exit 0 }
        $item.Delete()
    } else {
        Remove-Item -LiteralPath $link -Recurse -Force
    }
}
New-Item -ItemType Junction -Path $link -Target $target | Out-Null
]=])

set(ENV{DD_LINK_DIR} "${LINK_DIR}")
set(ENV{DD_TARGET_DIR} "${TARGET_DIR}")

execute_process(
    COMMAND powershell -NoProfile -ExecutionPolicy Bypass -Command "${_SCRIPT}"
    RESULT_VARIABLE _RESULT
)

if(NOT _RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to create junction: ${LINK_DIR} -> ${TARGET_DIR}")
endif()

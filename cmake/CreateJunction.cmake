# This script creates a junction (symbolic link) on Windows using PowerShell.
# Called at build time by add_custom_command via cmake -P.
# Required variables (passed with -D):
#   LINK_DIR   - path where the junction will be created
#   TARGET_DIR - path the junction points to

execute_process(
    COMMAND powershell -NoProfile -Command
        "Remove-Item -Force -Recurse -ErrorAction SilentlyContinue '${LINK_DIR}'; New-Item -ItemType Junction -Path '${LINK_DIR}' -Target '${TARGET_DIR}' | Out-Null"
    RESULT_VARIABLE _RESULT
)

if(NOT _RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to create junction: ${LINK_DIR} -> ${TARGET_DIR}")
endif()

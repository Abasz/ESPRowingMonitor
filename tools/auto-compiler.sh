#!/usr/bin/env bash
# Ensure this script is run with bash, not sh
set -o errexit
set -o nounset
set -o pipefail

RED=$'\e[0;31m'
GREEN=$'\e[0;32m'
BLUE=$'\e[0;34m'
YELLOW=$'\e[0;33m'
PURPLE=$'\e[0;35m'
NC=$'\e[0m' # No Color

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

# ─── PROFILE LISTS ────────────────────────────────────────────────
mapfile -t ROWERS < <(grep --only-matching --perl-regexp '^\[env:\K(?!.*(?:debug)).*(?=\])' < "${REPO_ROOT}/platformio.ini" | cut --delimiter '-' --fields 1 | sort --unique --merge)
mapfile -t BOARDS < <(sed --quiet --expression 's/^\[\(.*\)-board\]$/\1/p' < "${REPO_ROOT}/platformio.ini")

# ─── HELPERS ────────────────────────────────────────────────────────────────────
draw_splash() {
    echo "${YELLOW}"
cat <<'EOF'
 ________   ______   _______    _______                         _                    ____    ____                   _   _
|_   __  |.' ____ \ |_   __ \  |_   __ \                       (_)                  |_   \  /   _|                 (_) / |_
  | |_ \_|| (___ \_|  | |__) |   | |__) |   .--.   _   _   __  __   _ .--.   .--./)   |   \/   |   .--.   _ .--.   __ `| |-' .--.   _ .--.
  |  _| _  _.____`.   |  ___/    |  __ /  / .'`\ \[ \ [ \ [  ][  | [ `.-. | / /'`\;   | |\  /| | / .'`\ \[ `.-. | [  | | | / .'`\ \[ `/'`\]
 _| |__/ || \____) | _| |_      _| |  \ \_| \__. | \ \/\ \/ /  | |  | | | | \ \._//  _| |_\/_| |_| \__. | | | | |  | | | |,| \__. | | |
|________| \______.'|_____|    |____| |___|'.__.'   \__/\__/  [___][___||__].',__`  |_____||_____|'.__.' [___||__][___]\__/ '.__.' [___]
                                                                           ( ( __))
EOF
    echo "${PURPLE}Welcome to the ESP Rowing Monitor auto compiler!"
    echo "${NC}"
}

die() {
    echo "${RED}ERROR - $*${NC}" >&2
    exit 1
}

menu() {
    local -r prompt="$1"; shift 1
    local -r outRef="$1"; shift 1
    local -r options=("$@")
    local -i count=${#options[@]}
    local -i cur=0
    local -r esc=$'\e'
    
    tput civis # turn cursor off
    
    # print the prompt once
    echo "$prompt"
    
    while true; do
        # render
        for i in "${!options[@]}"; do
            if (( i == cur )); then
                # highlight the current choice (reverse video)
                printf " > %b%s%b\n" "${esc}[7m" "${options[i]}" "${esc}[0m"
            else
                printf "   %s\n" "${options[i]}"
            fi
        done
        
        # read first byte
        IFS= read -rsn1 key
        # if it's ESC, try to read up to 2 more bytes (non-blocking)
        if [[ $key == "$esc" ]]; then
            IFS= read -rsn2 -t 0.1 rest || true
            key+="${rest}"
        fi
        
        case "${key}" in
            "${esc}[A")  # up arrow
                (( cur > 0 )) && (( cur-- ))
            ;;
            "${esc}[B")  # down arrow
                (( cur < count - 1 )) && (( ++cur ))
            ;;
            "")        # ENTER pressed
                break
            ;;
        esac
        
        # move cursor up to re-draw exactly count lines
        printf "%b" "${esc}[${count}A"
    done
    
    tput cvvis # turn cursor on
    
    # output the selected value into the caller’s variable
    printf -v "${outRef}" '%s' "${options[cur]}"
}

is_valid_board() {
    local -r board="$1"; shift 1
    for valid in "${BOARDS[@]}"; do
        if [[ "${board}" == "${valid}" ]]; then
            return 0
        fi
    done
    return 1
}

get_rower_include_path() {
    local -r profile="$1"; shift 1
    local -r configPath="$1"; shift 1
    for rower in "${ROWERS[@]}"; do
        if [[ "${profile}" == "${rower}" ]]; then
            if [[ "${rower}" == "custom" ]]; then
                echo "$configPath"
                return 0
            fi
            echo "profiles/${rower}.rower-profile.h"
            return
        fi
    done
    die "Unknown rower profile '${profile}'"
}

extract_section() {
    local -r ini="$1"; shift 1
    local -r section="$1"; shift 1
    [[ -f "${ini}" ]] || die "File not found: ${ini}"
    [[ -n "${section}" ]] || die "Section name is required"
    # Escape any regex-special chars in section name
    local -r escSection=$(echo "${section}" | sed --expression 's/[][\.*^($){}+?|]/\\&/g')
    # Print from the section header down until (but not including) the next '[...'
    sed --quiet --expression "/^${escSection}$/,/^\[.*\]/p" "${ini}" | sed --expression '$d'
}

# ─── ARG PARSING ────────────────────────────────────────────────────────────────
ROWER=""
BOARD=""
CONFIG_FILE=""

usage() {
    
    local -r prog="$(basename "${0}")"
    local -r rowers="${ROWERS[*]}"
    local -r boards="${BOARDS[*]}"
    
    # CSS-like min-width for the 'Usage: $prog ' prefix
    local -r prefix="Usage: ${prog} "
    local -ri actualWidth=${#prefix}
    # Print first line normally
    printf "${GREEN}Usage: %s [-r, --rower <%s>] - Default: genericAir\n" "${prog}" "${rowers}"
    # Subsequent lines are indented to 'width'
    printf "%*s[-b, --board <%s>] - Default: generic\n" "${actualWidth}" "" "${boards}"
    printf "%*s[-f, --config-file <path-to-custom-h>] - Default: custom.settings.h\n\n${NC}" "${actualWidth}" ""
    cat <<EOF
If neither '--rower' nor '--board' is given, runs in interactive mode.
'--config-file' is ignored unless '--rower' is set to 'custom'.
EOF
    
    exit 1
}

while (( $# )); do
    case "$1" in
        -r|--rower)       ROWER="$2"; shift 2 ;;
        -f|--config-file) CONFIG_FILE="$2"; shift 2 ;;
        -b|--board)       BOARD="$2"; shift 2 ;;
        -o|--output-dir)  DEST_DIR="$2"; shift 2 ;;
        -h|--help)     usage ;;
        *)             die "Unknown argument: $1" ;;
    esac
done

draw_splash

# ─── INTERACTIVE MODE ─────────────────────────────────────────────────────────
if [[ -z "${ROWER}" && -z "${BOARD}" ]]; then
    echo "=== Interactive Mode ==="
    echo
    if [[ -z "${ROWER}" ]]; then
        menu "Select Rower Profile:" ROWER "${ROWERS[@]}"
        # if custom rower, prompt for header path
        if [[ "${ROWER}" == custom ]]; then
            read -rp "Enter path to custom rower .h file (custom.settings.h): " CONFIG_FILE
            CONFIG_FILE="${CONFIG_FILE:-custom.settings.h}"
        fi
    fi
    if [[ -z "${BOARD}" && -z "${CONFIG_FILE}" ]]; then
        echo
        menu "Select Board Profile:" BOARD "${BOARDS[@]}"
        echo
    fi
fi

# Validate custom files
if [[ "${ROWER}" == custom ]]; then
    : "${CONFIG_FILE:=custom.settings.h}"
    [[ -f "${CONFIG_FILE}" ]] || die "Config file not found: ${CONFIG_FILE}"
    
    BOARD=$(sed --quiet --expression 's/^\/\/[[:space:]]*board=\([[:alnum:]_-]\+\)/\1/p' "${CONFIG_FILE}")
    [[ -n "${BOARD}" ]] || die "Custom configuration must contain '// board=<board>' comment"
    if ! is_valid_board "${BOARD}"; then
        die "Invalid board '${BOARD}' in custom configuration. Valid board configurations are: $(printf '%s, ' "${BOARDS[@]}" | sed --expression 's/, $//')"
    fi
fi

# Apply defaults
: "${ROWER:=genericAir}"
: "${BOARD:=generic}"

# ─── SUMMARY ───────────────────────────────────────────────────────────────────
echo "Profiles → Rower: ${BLUE}${ROWER}${NC}; Board: ${BLUE}${BOARD}${NC}"
echo

# ─── GENERATE custom.settings.h ───────────────────────────────────────────────
SETTINGS_FILE="${REPO_ROOT}/src/custom.settings.h"
mkdir -p "$(dirname "${SETTINGS_FILE}")"

if ! is_valid_board "${BOARD}"; then
    die "Invalid board '${BOARD}'. Valid board configurations are: $(printf '%s, ' "${BOARDS[@]}" | sed --expression 's/, $//')"
fi

{
    echo "#pragma once"
    echo
    echo "#include BOARD_PROFILE"
    
    if [[ "${ROWER}" != custom ]]; then
        ROWER_INC=$(get_rower_include_path "${ROWER}" "${CONFIG_FILE}")
        echo "#include \"${ROWER_INC}\""
    fi
    
    echo
    echo "#include \"./utils/enums.h\""
    echo
    echo "// NOLINTBEGIN(cppcoreguidelines-macro-usage)"
    
    if [[ "${ROWER}" == custom ]]; then
        echo
        cat "${CONFIG_FILE}"
        echo
    fi
    
    echo
    echo "// NOLINTEND(cppcoreguidelines-macro-usage)"
} > "${SETTINGS_FILE}"

echo "Generated settings: ${BLUE}${SETTINGS_FILE}${NC}"

# ─── GENERATE platformio.custom.ini ───────────────────────────────────────────
MASTER_INI="${REPO_ROOT}/platformio.ini"
CUSTOM_INI="${REPO_ROOT}/platformio.custom.ini"

extract_section "${MASTER_INI}" "[env]" > "${CUSTOM_INI}"
extract_section "${MASTER_INI}" "[${BOARD}-board]" >> "${CUSTOM_INI}"
{
    echo "[env:custom]"
    echo "extends = ${BOARD}-board"
    echo "build_flags ="
    echo "    \${env.build_flags}"
    echo "    \${${BOARD}-board.build_flags}"
    echo "    '-D USE_CUSTOM_SETTINGS'"
} >> "${CUSTOM_INI}"

echo "Generated custom ini: ${BLUE}${CUSTOM_INI}${NC}"

# ─── ENSURE PYTHON 3 & PLATFORMIO ───────────────────────────────────────────────
if ! command -v pipx &>/dev/null; then
    echo "Installing pipx..."
    sudo apt-get update
    sudo apt-get install --yes pipx
fi
if ! command -v pio &>/dev/null; then
    echo "Installing PlatformIO..."
    pipx install platformio
    export PATH="${HOME}/.local/bin:${PATH}"
else
    echo "Checking PlatformIO version..."
    pio upgrade
fi

# ─── BUILD & COPY ───────────────────────────────────────────────────────────────
echo "Building firmware..."
pio run --project-conf "${CUSTOM_INI}" -e custom

SRC_BIN="${REPO_ROOT}/.pio/build/custom/firmware.bin"
DEST_BIN="${DEST_DIR:=$(pwd)}/firmware-${ROWER}-${BOARD}-$(date --iso-8601=date).bin"

if [[ -f "${SRC_BIN}" ]]; then
    cp "${SRC_BIN}" "${DEST_BIN}"
    echo "Firmware is at: ${BLUE}${DEST_BIN}${NC}"
else
    die "Build failed: ${SRC_BIN} not found"
fi

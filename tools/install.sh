#!/usr/bin/env bash
set -o errexit
set -o nounset
set -o pipefail

GIT_REPO_URL="https://github.com/Abasz/Test.git"
GIT_TAG="latest"

RED=$'\e[0;31m'
YELLOW=$'\e[0;33m'
PURPLE=$'\e[0;35m'
NC=$'\e[0m' # No Color

# ─── HELPERS ─────────────────────────────────────────────────────────────────
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
    echo "${PURPLE}Welcome to the ESP Rowing Monitor installer!${NC}"
}

usage() {
    cat <<EOF
Usage: $(basename "$0") [<tag>]

Positional Arguments:
  <tag>                Git tag or branch to checkout. Defaults to 'latest'.
                       Use 'current' to stay on the current working directory without cloning or checking out.

Options:
  -h, --help           Show this help message and exit.
EOF
}

die() {
    echo "${RED}ERROR - $*${NC}" >&2
    exit 1
}

run_auto_compiler() {
    local -r baseDir="$1"; shift 1
    local autoCompilerArgs=()
    local -r currentWorkingDir="$(pwd)"
    
    
    read -rp "Do you want to run the auto-compiler script now? (Y/n): " runCompiler
    if [[ -z "$runCompiler" || "$runCompiler" =~ ^[Yy]$ ]]; then
        [[ -f "custom.settings.h" ]] && autoCompilerArgs=("--rower" "custom")
        
        pushd "${baseDir}" >/dev/null || die "Failed to enter directory ${baseDir}"
        
        ./tools/auto-compiler.sh "${autoCompilerArgs[@]}" --output-dir "${currentWorkingDir}"
        
        popd >/dev/null
        
        exit 0
    fi
    
    echo "Skipping auto-compiler script."
}

# ─── ARGUMENT PARSING ─────────────────────────────────────────────
draw_splash

if [[ $# -gt 0 ]]; then
    case "$1" in
        -h|--help)
            usage
            exit 0
        ;;
        *)
            GIT_TAG=$1
        ;;
    esac
fi

# ─── DETERMINE SCRIPT LOCATION ─────────────────────────────────────────────────
[[ -n "${BASH_SOURCE[0]:-}" ]] && SCRIPT_DIR_FULL_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)" || SCRIPT_DIR_FULL_PATH="$(pwd)"
SCRIPT_PARENT_DIR="$(basename "${SCRIPT_DIR_FULL_PATH}")"

# ─── SKIP EVERYTHING IF SCRIPT IS IN 'tools' AND TAG IS 'current' ───────────────
if [[ "${SCRIPT_PARENT_DIR}" == "tools" && "${GIT_TAG}" == "current" ]]; then
    echo
    echo "Installation is complete"
    run_auto_compiler
fi

# ─── GIT INSTALL & CLONE/CHECKOUT LOGIC ─────────────────────────────────────
# Ensure git is installed
if ! command -v git &>/dev/null; then
    echo
    echo "Git not found, installing..."
    sudo apt-get update
    sudo apt-get install --yes git
fi

# Clone repository and checkout specific tag or latest
[[ "${SCRIPT_PARENT_DIR}" == "tools" ]] && CHECKOUT_DIR="${SCRIPT_DIR_FULL_PATH}/.." || CHECKOUT_DIR=$(basename -s .git "${GIT_REPO_URL}")

# Clone only if not in 'tools' directory and repo is not already cloned
if [[ "${SCRIPT_PARENT_DIR}" != "tools" && ! -d "${CHECKOUT_DIR}/.git" ]]; then
    echo
    echo "Installing repository from ${GIT_REPO_URL}"
    git clone "${GIT_REPO_URL}" "${CHECKOUT_DIR}" || die "Failed to clone ${GIT_REPO_URL}"
fi

# Checkout logic if GIT_TAG is not 'current'
if [[ "${GIT_TAG}" != "current" ]]; then
    echo
    pushd "${CHECKOUT_DIR}" >/dev/null || die "Failed to enter directory ${CHECKOUT_DIR}"
    git fetch --all --tags
    git pull
    
    if [[ "${GIT_TAG}" == "latest" ]]; then
        GIT_TAG=$(git tag --sort=-creatordate | head -n 1)
    fi
    
    echo "Checking out tag: ${GIT_TAG} in directory: ${CHECKOUT_DIR}"
    
    git reset --hard "${GIT_TAG}" || die "Failed to checkout tag ${GIT_TAG}"
    popd >/dev/null
fi

# ─── SET BASE DIRECTORY ─────────────────────────────────────────────────────────
echo
echo "Base directory set to: ${CHECKOUT_DIR:-.}"
echo

run_auto_compiler "${CHECKOUT_DIR:-.}"
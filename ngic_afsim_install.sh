#!/usr/bin/env bash
# ****************************************************************************
# NGIC AFSIM Plugin Install Script
# ****************************************************************************
#
# Generic install script for AFSIM plugins. Auto-detects AFSIM installation
# and installs all plugins found in the lib/ directory.
#
# IMPORTANT: AFSIM loads plugins from TWO directories:
#   1. $AFSIM_DIR/lib/wsf_plugins/      (primary)
#   2. $AFSIM_DIR/bin/mission_plugins/  (secondary)
#
# This script installs to BOTH locations to ensure plugins are loaded.
#
# Usage:
#   ./ngic_afsim_install.sh              # Install all plugins
#   ./ngic_afsim_install.sh --uninstall  # Remove plugins
#   ./ngic_afsim_install.sh --verify     # Verify installation
#   ./ngic_afsim_install.sh --help       # Show help
#
# Prerequisites:
#   - Run ngic_afsim_build.sh first to compile plugins
#   - sudo access for installation
#
# Directory Structure Expected:
#   project_root/
#   ├── lib/                         (compiled plugins from build script)
#   │   ├── libplugin1.so
#   │   └── libplugin2.so
#   └── ngic_afsim_install.sh        (this script)
#
# Developed by: US Army National Ground Intelligence Center (NGIC)
# ****************************************************************************

set -e  # Exit on error

# =============================================================================
# CONFIGURATION
# =============================================================================

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# Script location and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LIB_DIR="${SCRIPT_DIR}/lib"

# =============================================================================
# FUNCTIONS
# =============================================================================

print_banner() {
    echo -e "${CYAN}"
    echo "===================================================================================="
    echo "                    NGIC AFSIM Plugin Install Script"
    echo "===================================================================================="
    echo -e "${NC}"
}

print_help() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  (none)        Install all plugins to AFSIM"
    echo "  --uninstall   Remove project plugins from AFSIM"
    echo "  --verify      Verify plugin installation (checksums)"
    echo "  --list        List plugins that would be installed"
    echo "  --help        Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                    # Install plugins"
    echo "  $0 --verify           # Check installation"
    echo "  $0 --uninstall        # Remove plugins"
    echo ""
    echo "Note: This script requires sudo access to install to AFSIM directories."
    echo ""
}

find_all_afsim() {
    # Find all AFSIM installations and return them as a newline-separated list
    local candidates=()
    local search_dirs=(/opt /usr/local "${HOME}")

    for base_dir in "${search_dirs[@]}"; do
        [ -d "$base_dir" ] || continue

        for path in "$base_dir"/afsim* "$base_dir"/AFSIM*; do
            [ -d "$path" ] || continue
            [ -d "$path/include" ] || continue
            [ -d "$path/lib" ] || continue

            # Verify it's a valid AFSIM installation
            if [ -f "$path/lib/cmake/AFSIM/AFSIMConfig.cmake" ] || \
               [ -d "$path/lib/wsf_plugins" ] || \
               [ -f "$path/bin/mission" ]; then
                candidates+=("$path")
            fi
        done
    done

    # Sort candidates by version (highest first)
    IFS=$'\n' sorted=($(sort -r <<<"${candidates[*]}")); unset IFS

    for c in "${sorted[@]}"; do
        echo "$c"
    done
}

get_afsim_version() {
    # Get AFSIM version from installation directory
    local afsim_dir="$1"
    local version="unknown"

    # Try lowercase path first (AFSIM 2.2505+)
    if [ -f "${afsim_dir}/lib/cmake/afsim/afsim-config-version.cmake" ]; then
        version=$(grep -oP 'set\s*\(\s*PACKAGE_VERSION\s+"\K[^"]+' "${afsim_dir}/lib/cmake/afsim/afsim-config-version.cmake" 2>/dev/null | head -1 || echo "unknown")
    # Try uppercase path (older versions)
    elif [ -f "${afsim_dir}/lib/cmake/AFSIM/AFSIMConfigVersion.cmake" ]; then
        version=$(grep -oP 'PACKAGE_VERSION\s+"\K[^"]+' "${afsim_dir}/lib/cmake/AFSIM/AFSIMConfigVersion.cmake" 2>/dev/null || echo "unknown")
    # Try version file
    elif [ -f "${afsim_dir}/lib/cmake/afsim/version" ]; then
        version=$(cat "${afsim_dir}/lib/cmake/afsim/version" 2>/dev/null || echo "unknown")
    fi

    echo "$version"
}

select_afsim() {
    # Find all AFSIM installations and let user select if multiple found
    # If AFSIM_ROOT or AFSIM_DIR env var is set, use that directly

    # Check environment variables first
    for env_path in "${AFSIM_ROOT:-}" "${AFSIM_DIR:-}"; do
        if [ -n "$env_path" ] && [ -d "$env_path" ] && [ -d "$env_path/include" ]; then
            if [ -f "$env_path/lib/cmake/AFSIM/AFSIMConfig.cmake" ] || \
               [ -d "$env_path/lib/wsf_plugins" ] || \
               [ -f "$env_path/bin/mission" ]; then
                echo "$env_path"
                return 0
            fi
        fi
    done

    # Find all installations
    local installations=()
    while IFS= read -r line; do
        [ -n "$line" ] && installations+=("$line")
    done < <(find_all_afsim)

    if [ ${#installations[@]} -eq 0 ]; then
        return 1
    elif [ ${#installations[@]} -eq 1 ]; then
        echo "${installations[0]}"
        return 0
    fi

    # Multiple installations found
    # Check if we're running interactively
    if [ -t 0 ]; then
        # Interactive mode - prompt user
        echo -e "${YELLOW}Multiple AFSIM installations found:${NC}" >&2
        echo "" >&2

        local i=1
        for install_path in "${installations[@]}"; do
            local version=$(get_afsim_version "$install_path")
            echo -e "  ${BOLD}${i})${NC} ${install_path}" >&2
            echo -e "     Version: ${version}" >&2
            ((i++))
        done

        echo "" >&2
        local max_attempts=3
        local attempt=0
        while [ $attempt -lt $max_attempts ]; do
            read -p "Select AFSIM installation [1-${#installations[@]}] (default: 1): " choice
            # Default to 1 if empty
            [ -z "$choice" ] && choice=1
            if [[ "$choice" =~ ^[0-9]+$ ]] && [ "$choice" -ge 1 ] && [ "$choice" -le ${#installations[@]} ]; then
                echo "${installations[$((choice-1))]}"
                return 0
            else
                echo -e "${RED}Invalid choice. Please enter a number between 1 and ${#installations[@]}.${NC}" >&2
                ((attempt++))
            fi
        done
        # After max attempts, use default
        echo -e "${YELLOW}Using default: ${installations[0]}${NC}" >&2
        echo "${installations[0]}"
        return 0
    else
        # Non-interactive mode - use the first (highest version) automatically
        echo -e "${YELLOW}Multiple AFSIM installations found (non-interactive mode):${NC}" >&2
        for install_path in "${installations[@]}"; do
            local version=$(get_afsim_version "$install_path")
            echo -e "  - ${install_path} (${version})" >&2
        done
        echo -e "${YELLOW}Using: ${installations[0]}${NC}" >&2
        echo -e "${YELLOW}Set AFSIM_ROOT environment variable to override.${NC}" >&2
        echo "${installations[0]}"
        return 0
    fi
}

discover_local_plugins() {
    # Find all .so files in the lib/ directory AND plugins/build/lib/
    # Prefers lib-prefixed versions to avoid duplicates
    # Also syncs any missing plugins from build/lib to lib/

    local plugins=()
    local seen_bases=()
    local BUILD_LIB_DIR="${SCRIPT_DIR}/plugins/build/lib"

    # First, sync any missing plugins from build/lib to lib/
    if [ -d "${BUILD_LIB_DIR}" ] && [ -d "${LIB_DIR}" ]; then
        for so_file in "${BUILD_LIB_DIR}"/*.so; do
            [ -f "$so_file" ] || continue
            local filename=$(basename "$so_file")
            local dest_name="${filename}"
            [[ "$filename" != lib* ]] && dest_name="lib${filename}"

            # Copy if not exists or if build version is newer
            if [ ! -f "${LIB_DIR}/${dest_name}" ] || [ "$so_file" -nt "${LIB_DIR}/${dest_name}" ]; then
                cp "$so_file" "${LIB_DIR}/${dest_name}"
                echo -e "${YELLOW}  Synced: ${filename} -> ${dest_name}${NC}" >&2
            fi
        done
    elif [ -d "${BUILD_LIB_DIR}" ] && [ ! -d "${LIB_DIR}" ]; then
        # lib/ doesn't exist, create it and copy all plugins
        mkdir -p "${LIB_DIR}"
        for so_file in "${BUILD_LIB_DIR}"/*.so; do
            [ -f "$so_file" ] || continue
            local filename=$(basename "$so_file")
            local dest_name="${filename}"
            [[ "$filename" != lib* ]] && dest_name="lib${filename}"
            cp "$so_file" "${LIB_DIR}/${dest_name}"
            echo -e "${YELLOW}  Synced: ${filename} -> ${dest_name}${NC}" >&2
        done
    fi

    # Now discover plugins from lib/
    if [ -d "${LIB_DIR}" ]; then
        # Use find for reliable file discovery
        while IFS= read -r so_file; do
            [ -f "$so_file" ] || continue
            local filename=$(basename "$so_file")

            # Track base name to avoid duplicates
            local base="${filename#lib}"
            local already_seen=false
            for seen in "${seen_bases[@]}"; do
                if [ "$base" = "$seen" ] || [ "$filename" = "$seen" ]; then
                    already_seen=true
                    break
                fi
            done

            if ! $already_seen; then
                plugins+=("$filename")
                seen_bases+=("$base")
            fi
        done < <(find "${LIB_DIR}" -maxdepth 1 -name "*.so" -type f 2>/dev/null | sort)
    fi

    echo "${plugins[@]}"
}

get_project_name() {
    # Try to determine project name
    local project_name=""

    if [ -f "${SCRIPT_DIR}/plugins/CMakeLists.txt" ]; then
        project_name=$(grep -oP 'project\s*\(\s*\K[^\s)]+' "${SCRIPT_DIR}/plugins/CMakeLists.txt" 2>/dev/null | head -1)
    fi

    if [ -z "$project_name" ]; then
        project_name=$(basename "${SCRIPT_DIR}")
    fi

    echo "$project_name"
}

install_plugins() {
    local afsim_dir="$1"
    shift
    local plugins=("$@")

    local wsf_plugins_dir="${afsim_dir}/lib/wsf_plugins"
    local mission_plugins_dir="${afsim_dir}/bin/mission_plugins"

    # Ensure directories exist
    if [ ! -d "$wsf_plugins_dir" ]; then
        echo -e "${YELLOW}Creating ${wsf_plugins_dir}...${NC}"
        sudo mkdir -p "$wsf_plugins_dir"
    fi

    if [ ! -d "$mission_plugins_dir" ]; then
        echo -e "${YELLOW}Creating ${mission_plugins_dir}...${NC}"
        sudo mkdir -p "$mission_plugins_dir"
    fi

    echo ""
    echo -e "${BLUE}Installing to ${wsf_plugins_dir}...${NC}"
    for plugin in "${plugins[@]}"; do
        local src="${LIB_DIR}/${plugin}"
        local dest="${wsf_plugins_dir}/${plugin}"

        # For wsf_plugins, some files don't need lib prefix
        local dest_name="${plugin}"
        if [[ "$plugin" == lib* ]]; then
            # Remove lib prefix for wsf_plugins (AFSIM convention varies)
            dest_name="${plugin#lib}"
        fi

        echo -e "  Installing ${plugin}..."
        sudo cp "$src" "${wsf_plugins_dir}/${dest_name}"
        sudo chmod 755 "${wsf_plugins_dir}/${dest_name}"
        echo -e "${GREEN}  ✓ Installed ${dest_name}${NC}"
    done

    echo ""
    echo -e "${BLUE}Installing to ${mission_plugins_dir}...${NC}"
    for plugin in "${plugins[@]}"; do
        local src="${LIB_DIR}/${plugin}"

        # mission_plugins expects lib prefix
        local dest_name="${plugin}"
        if [[ "$plugin" != lib* ]]; then
            dest_name="lib${plugin}"
        fi

        echo -e "  Installing ${plugin}..."
        sudo cp "$src" "${mission_plugins_dir}/${dest_name}"
        sudo chmod 755 "${mission_plugins_dir}/${dest_name}"
        echo -e "${GREEN}  ✓ Installed ${dest_name}${NC}"
    done
}

uninstall_plugins() {
    local afsim_dir="$1"
    shift
    local plugins=("$@")

    local wsf_plugins_dir="${afsim_dir}/lib/wsf_plugins"
    local mission_plugins_dir="${afsim_dir}/bin/mission_plugins"

    echo -e "${YELLOW}Uninstalling plugins...${NC}"

    for plugin in "${plugins[@]}"; do
        # Try both with and without lib prefix
        local names=("$plugin" "${plugin#lib}" "lib${plugin}")

        for dir in "$wsf_plugins_dir" "$mission_plugins_dir"; do
            for name in "${names[@]}"; do
                if [ -f "${dir}/${name}" ]; then
                    echo -e "  Removing ${dir}/${name}..."
                    sudo rm -f "${dir}/${name}"
                    echo -e "${GREEN}  ✓ Removed${NC}"
                fi
            done
        done
    done
}

verify_plugins() {
    local afsim_dir="$1"
    shift
    local plugins=("$@")

    local wsf_plugins_dir="${afsim_dir}/lib/wsf_plugins"
    local mission_plugins_dir="${afsim_dir}/bin/mission_plugins"

    echo -e "${BLUE}Verifying plugin installation...${NC}"
    echo ""

    local all_ok=true

    for plugin in "${plugins[@]}"; do
        local src="${LIB_DIR}/${plugin}"
        local src_md5=$(md5sum "$src" 2>/dev/null | awk '{print $1}')

        echo -e "${CYAN}${plugin}:${NC}"
        echo -e "  Source:  ${src_md5}"

        # Check wsf_plugins
        local dest_name="${plugin#lib}"  # Remove lib prefix
        local dest="${wsf_plugins_dir}/${dest_name}"
        if [ -f "$dest" ]; then
            local dest_md5=$(md5sum "$dest" 2>/dev/null | awk '{print $1}')
            if [ "$src_md5" = "$dest_md5" ]; then
                echo -e "  ${GREEN}✓ wsf_plugins: ${dest_md5}${NC}"
            else
                echo -e "  ${RED}✗ wsf_plugins: ${dest_md5} (MISMATCH)${NC}"
                all_ok=false
            fi
        else
            echo -e "  ${RED}✗ wsf_plugins: NOT FOUND${NC}"
            all_ok=false
        fi

        # Check mission_plugins
        local dest_name2="${plugin}"
        [[ "$plugin" != lib* ]] && dest_name2="lib${plugin}"
        local dest2="${mission_plugins_dir}/${dest_name2}"
        if [ -f "$dest2" ]; then
            local dest2_md5=$(md5sum "$dest2" 2>/dev/null | awk '{print $1}')
            if [ "$src_md5" = "$dest2_md5" ]; then
                echo -e "  ${GREEN}✓ mission_plugins: ${dest2_md5}${NC}"
            else
                echo -e "  ${RED}✗ mission_plugins: ${dest2_md5} (MISMATCH)${NC}"
                all_ok=false
            fi
        else
            echo -e "  ${RED}✗ mission_plugins: NOT FOUND${NC}"
            all_ok=false
        fi

        echo ""
    done

    if $all_ok; then
        echo -e "${GREEN}All plugins verified successfully!${NC}"
        return 0
    else
        echo -e "${RED}Some plugins failed verification.${NC}"
        return 1
    fi
}

# =============================================================================
# MAIN SCRIPT
# =============================================================================

print_banner

# Handle arguments
ACTION="install"
case "${1:-}" in
    --help|-h)
        print_help
        exit 0
        ;;
    --uninstall|--remove)
        ACTION="uninstall"
        ;;
    --verify|--check)
        ACTION="verify"
        ;;
    --list)
        ACTION="list"
        ;;
    "")
        ACTION="install"
        ;;
    *)
        echo -e "${RED}Unknown option: $1${NC}"
        print_help
        exit 1
        ;;
esac

# Find AFSIM installation
echo -e "${BLUE}Searching for AFSIM installation...${NC}"
AFSIM_DIR=$(select_afsim) || {
    echo -e "${RED}ERROR: Could not find AFSIM installation${NC}"
    echo ""
    echo "Searched locations:"
    echo "  - \$AFSIM_ROOT environment variable"
    echo "  - /opt/afsim*"
    echo "  - /usr/local/afsim*"
    echo "  - ~/afsim*"
    echo ""
    echo "Please set AFSIM_ROOT environment variable or install AFSIM to a standard location."
    exit 1
}
echo -e "${GREEN}✓ Found AFSIM: ${AFSIM_DIR}${NC}"

# Discover local plugins
PLUGINS=($(discover_local_plugins))

if [ ${#PLUGINS[@]} -eq 0 ]; then
    echo -e "${RED}ERROR: No plugins found in ${LIB_DIR}${NC}"
    echo ""
    echo "Please run the build script first:"
    echo "  ./ngic_afsim_build.sh"
    exit 1
fi

echo -e "${GREEN}✓ Found ${#PLUGINS[@]} plugin(s) to install${NC}"

# Get project name
PROJECT_NAME=$(get_project_name)

# Print configuration
echo ""
echo -e "${CYAN}===================================================================================="
echo "Installation Configuration"
echo "====================================================================================${NC}"
echo -e "  Project:          ${BOLD}${PROJECT_NAME}${NC}"
echo -e "  AFSIM Directory:  ${AFSIM_DIR}"
echo -e "  Source Directory: ${LIB_DIR}"
echo -e "  Plugins:          ${PLUGINS[*]}"
echo ""
echo -e "  Install Targets:"
echo -e "    - ${AFSIM_DIR}/lib/wsf_plugins/"
echo -e "    - ${AFSIM_DIR}/bin/mission_plugins/"
echo ""

# Execute action
case "$ACTION" in
    list)
        echo -e "${BLUE}Plugins to be installed:${NC}"
        for plugin in "${PLUGINS[@]}"; do
            size=$(ls -lh "${LIB_DIR}/${plugin}" | awk '{print $5}')
            echo -e "  ${plugin} (${size})"
        done
        ;;

    verify)
        verify_plugins "$AFSIM_DIR" "${PLUGINS[@]}"
        ;;

    uninstall)
        echo -e "${YELLOW}This will remove the following plugins from AFSIM:${NC}"
        for plugin in "${PLUGINS[@]}"; do
            echo -e "  ${plugin}"
        done
        echo ""
        read -p "Continue? [y/N] " confirm
        if [[ "$confirm" =~ ^[Yy]$ ]]; then
            uninstall_plugins "$AFSIM_DIR" "${PLUGINS[@]}"
            echo ""
            echo -e "${GREEN}Plugins uninstalled successfully!${NC}"
        else
            echo "Aborted."
            exit 0
        fi
        ;;

    install)
        echo -e "${YELLOW}Installing plugins (sudo required)...${NC}"
        echo ""
        install_plugins "$AFSIM_DIR" "${PLUGINS[@]}"

        # Verify installation
        echo ""
        echo -e "${CYAN}===================================================================================="
        echo "Verifying Installation"
        echo "====================================================================================${NC}"
        echo ""
        verify_plugins "$AFSIM_DIR" "${PLUGINS[@]}" || true

        # Print success message
        echo ""
        echo -e "${GREEN}===================================================================================="
        echo "Installation Complete!"
        echo "====================================================================================${NC}"
        echo ""
        echo "Plugins are now available in AFSIM. To test:"
        echo ""
        echo "  # Run a scenario"
        echo "  ${AFSIM_DIR}/bin/mission scenarios/your_scenario.afsim"
        echo ""
        echo "  # Run with visualization"
        echo "  DISPLAY=:0 ${AFSIM_DIR}/bin/warlock scenarios/your_scenario.afsim"
        echo ""
        ;;
esac

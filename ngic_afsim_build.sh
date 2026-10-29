#!/usr/bin/env bash
# ****************************************************************************
# NGIC AFSIM Plugin Build Script
# ****************************************************************************
#
# Generic build script for AFSIM plugins. Auto-detects AFSIM installation
# and discovers plugins in the plugins/ directory.
#
# Usage:
#   ./ngic_afsim_build.sh [debug|release]
#   ./ngic_afsim_build.sh --clean          # Clean build directory
#   ./ngic_afsim_build.sh --help           # Show help
#
# Prerequisites:
#   - AFSIM 2.2505.x LTS installed
#   - CMake 3.16 or later
#   - GCC/G++ with C++17 support
#
# Directory Structure Expected:
#   project_root/
#   ├── plugins/
#   │   ├── CMakeLists.txt
#   │   ├── plugin_name_1/
#   │   │   └── source/
#   │   └── plugin_name_2/
#   │       └── source/
#   ├── lib/                    (created by this script)
#   └── ngic_afsim_build.sh     (this script)
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
PLUGINS_DIR="${SCRIPT_DIR}/plugins"
BUILD_DIR="${PLUGINS_DIR}/build"
LIB_DIR="${SCRIPT_DIR}/lib"

# Build configuration
BUILD_TYPE="${1:-Release}"

# =============================================================================
# FUNCTIONS
# =============================================================================

print_banner() {
    echo -e "${CYAN}"
    echo "===================================================================================="
    echo "                    NGIC AFSIM Plugin Build Script"
    echo "===================================================================================="
    echo -e "${NC}"
}

print_help() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  debug       Build with debug symbols"
    echo "  release     Build optimized release (default)"
    echo "  --clean     Remove build directory and rebuild"
    echo "  --help      Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0              # Release build"
    echo "  $0 debug        # Debug build"
    echo "  $0 --clean      # Clean and rebuild"
    echo ""
}

find_all_afsim() {
    # Find all AFSIM installations and return them as a newline-separated list
    # Returns: list of valid AFSIM installation paths

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

discover_plugins() {
    # Discover plugin directories in plugins/ folder
    # A valid plugin directory contains a CMakeLists.txt or source/ subdirectory

    local plugins_found=()

    if [ ! -d "${PLUGINS_DIR}" ]; then
        return 1
    fi

    for dir in "${PLUGINS_DIR}"/*/; do
        [ -d "$dir" ] || continue
        local name=$(basename "$dir")

        # Skip build directory and hidden directories
        [[ "$name" == "build" ]] && continue
        [[ "$name" == .* ]] && continue

        # Check if it's a valid plugin directory
        if [ -d "$dir/source" ] || [ -f "$dir/CMakeLists.txt" ]; then
            plugins_found+=("$name")
        fi
    done

    echo "${plugins_found[@]}"
}

get_project_name() {
    # Try to determine project name from directory or CMakeLists.txt
    local project_name=""

    # Try to extract from CMakeLists.txt
    if [ -f "${PLUGINS_DIR}/CMakeLists.txt" ]; then
        project_name=$(grep -oP 'project\s*\(\s*\K[^\s)]+' "${PLUGINS_DIR}/CMakeLists.txt" 2>/dev/null | head -1)
    fi

    # Fall back to parent directory name
    if [ -z "$project_name" ]; then
        project_name=$(basename "${SCRIPT_DIR}")
    fi

    echo "$project_name"
}

# =============================================================================
# MAIN SCRIPT
# =============================================================================

print_banner

# Handle arguments
case "${1:-}" in
    --help|-h)
        print_help
        exit 0
        ;;
    --clean)
        echo -e "${YELLOW}Cleaning build directory...${NC}"
        rm -rf "${BUILD_DIR}"
        rm -rf "${LIB_DIR}"
        echo -e "${GREEN}✓ Clean complete${NC}"
        BUILD_TYPE="Release"
        ;;
    debug|Debug)
        BUILD_TYPE="Debug"
        ;;
    release|Release|"")
        BUILD_TYPE="Release"
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

# Get AFSIM version
AFSIM_VERSION=$(get_afsim_version "$AFSIM_DIR")
echo -e "${GREEN}✓ AFSIM Version: ${AFSIM_VERSION}${NC}"

# Check for plugins directory
echo ""
echo -e "${BLUE}Checking project structure...${NC}"
if [ ! -d "${PLUGINS_DIR}" ]; then
    echo -e "${RED}ERROR: plugins/ directory not found${NC}"
    echo "Expected directory structure:"
    echo "  ${SCRIPT_DIR}/"
    echo "  └── plugins/"
    echo "      ├── CMakeLists.txt"
    echo "      └── plugin_name/"
    exit 1
fi

# Check for CMakeLists.txt
if [ ! -f "${PLUGINS_DIR}/CMakeLists.txt" ]; then
    echo -e "${RED}ERROR: plugins/CMakeLists.txt not found${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Found plugins/CMakeLists.txt${NC}"

# Discover plugins
PLUGINS=($(discover_plugins))
if [ ${#PLUGINS[@]} -eq 0 ]; then
    echo -e "${YELLOW}WARNING: No plugin directories found in plugins/${NC}"
else
    echo -e "${GREEN}✓ Found ${#PLUGINS[@]} plugin(s): ${PLUGINS[*]}${NC}"
fi

# Get project name
PROJECT_NAME=$(get_project_name)

# Print configuration
echo ""
echo -e "${CYAN}===================================================================================="
echo "Build Configuration"
echo "====================================================================================${NC}"
echo -e "  Project:          ${BOLD}${PROJECT_NAME}${NC}"
echo -e "  AFSIM Directory:  ${AFSIM_DIR}"
echo -e "  AFSIM Version:    ${AFSIM_VERSION}"
echo -e "  Build Type:       ${BUILD_TYPE}"
echo -e "  Build Directory:  ${BUILD_DIR}"
echo -e "  Output Directory: ${LIB_DIR}"
echo -e "  Plugins:          ${PLUGINS[*]}"
echo ""

# Create build directory
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Configure CMake
echo -e "${BLUE}Configuring CMake...${NC}"
cmake \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_PREFIX_PATH="${AFSIM_DIR}" \
    -DCMAKE_INSTALL_PREFIX="${AFSIM_DIR}" \
    -DAFSIM_ROOT="${AFSIM_DIR}" \
    "${PLUGINS_DIR}"

# Build
echo ""
echo -e "${BLUE}Building plugins...${NC}"
cmake --build . --parallel $(nproc)

# Copy built plugins to lib/ directory
echo ""
echo -e "${BLUE}Copying plugins to lib/ directory...${NC}"
mkdir -p "${LIB_DIR}"

# Find all .so files in build/lib and copy with lib prefix
COPIED_COUNT=0
if [ -d "${BUILD_DIR}/lib" ]; then
    # Use find to get all .so files reliably
    while IFS= read -r so_file; do
        [ -f "$so_file" ] || continue

        filename=$(basename "$so_file")
        # Add lib prefix if not present
        if [[ "$filename" != lib* ]]; then
            dest_name="lib${filename}"
        else
            dest_name="$filename"
        fi

        cp "$so_file" "${LIB_DIR}/${dest_name}"
        echo -e "${GREEN}  ✓ ${filename} -> ${dest_name}${NC}"
        COPIED_COUNT=$((COPIED_COUNT + 1))
    done < <(find "${BUILD_DIR}/lib" -maxdepth 1 -name "*.so" -type f 2>/dev/null)
fi

# Summary
echo ""
echo -e "${CYAN}===================================================================================="
echo "Build Complete"
echo "====================================================================================${NC}"
echo ""
echo -e "${GREEN}Built ${COPIED_COUNT} plugin(s):${NC}"
ls -lh "${LIB_DIR}"/*.so 2>/dev/null || echo "  (No .so files found)"
echo ""
echo -e "${YELLOW}Next Steps:${NC}"
echo "  To install plugins to AFSIM, run:"
echo -e "    ${BOLD}./ngic_afsim_install.sh${NC}"
echo ""
echo "  Or install manually with sudo:"
echo "    sudo cmake --install ${BUILD_DIR}"
echo ""

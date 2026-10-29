#!/bin/bash
# ****************************************************************************
# T-150 UAS Plugins - Build Script
#
# This script builds the T-150 custom plugins against the installed AFSIM.
#
# Usage: ./build.sh [debug|release]
#
# Prerequisites:
# - AFSIM 2.2505 installed at /opt/afsim2.2505
# - CMake 3.16 or later
# - GCC/G++ with C++17 support
#
# Copyright 2025 - Project 005 T-150 UAS Simulation
# ****************************************************************************

set -e

# Configuration
AFSIM_DIR="/opt/afsim2.2505"
BUILD_TYPE="${1:-Release}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
INSTALL_DIR="${AFSIM_DIR}/lib/wsf_plugins"

echo "=============================================="
echo "T-150 UAS Plugins Build"
echo "=============================================="
echo "AFSIM Directory: ${AFSIM_DIR}"
echo "Build Type: ${BUILD_TYPE}"
echo "Build Directory: ${BUILD_DIR}"
echo "Install Directory: ${INSTALL_DIR}"
echo ""

# Check AFSIM installation
if [ ! -d "${AFSIM_DIR}" ]; then
    echo "ERROR: AFSIM not found at ${AFSIM_DIR}"
    exit 1
fi

if [ ! -d "${AFSIM_DIR}/include" ]; then
    echo "ERROR: AFSIM include directory not found"
    exit 1
fi

# Create build directory
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Configure CMake
echo "Configuring CMake..."
cmake \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_PREFIX_PATH="${AFSIM_DIR}" \
    -DCMAKE_INSTALL_PREFIX="${AFSIM_DIR}" \
    -DAFSIM_ROOT="${AFSIM_DIR}" \
    "${SCRIPT_DIR}"

# Build
echo "Building..."
cmake --build . --parallel $(nproc)

# Copy built plugins to top-level lib/ with correct naming for install script
echo ""
echo "Copying plugins to lib/ directory..."
PROJECT_LIB="${SCRIPT_DIR}/../lib"
mkdir -p "${PROJECT_LIB}"

# Copy with lib prefix for install script compatibility
for plugin in t150_battery_processor t150_vrs t150_propulsion t150_slung_load rotorcraft_6dof; do
    if [ -f "${BUILD_DIR}/lib/${plugin}.so" ]; then
        cp "${BUILD_DIR}/lib/${plugin}.so" "${PROJECT_LIB}/lib${plugin}.so"
        echo "  Copied ${plugin}.so -> lib${plugin}.so"
    fi
done

echo ""
echo "Build complete!"
echo ""
echo "To install the plugins to AFSIM, run:"
echo "  sudo cmake --install ${BUILD_DIR}"
echo ""
echo "Or copy the .so files manually:"
echo "  sudo cp ${BUILD_DIR}/*.so ${INSTALL_DIR}/"
echo ""
echo "Plugin libraries built:"
ls -la "${BUILD_DIR}/lib"/*.so 2>/dev/null || echo "  (No .so files found - check build output)"

#!/bin/bash
# Source67 Engine - Hybrid Build System Build Script
# Builds engine, game DLL, and asset pack

set -e

BUILD_TYPE=${1:-Release}
TARGET=${2:-all}

echo "========================================="
echo "Source67 Engine - Hybrid Build System"
echo "========================================="
echo "Build Type: $BUILD_TYPE"
echo "Target: $TARGET"
echo ""

# Step 1: Build Game.dll
if [ "$TARGET" = "all" ] || [ "$TARGET" = "game" ]; then
    echo "[1/3] Building Game.dll..."
    cd game
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -B build
    cmake --build build --config $BUILD_TYPE
    echo "✓ Game.dll compiled"
    cd ..
    echo ""
fi

# Step 2: Build Asset Packer Tool
if [ "$TARGET" = "all" ] || [ "$TARGET" = "tools" ]; then
    echo "[2/3] Building asset packer tool..."
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DBUILD_ASSET_PACKER=ON -B cmake-build-tools
    cmake --build cmake-build-tools --target asset_packer --config $BUILD_TYPE
    echo "✓ Asset packer built"
    echo ""
fi

# Step 3: Pack Assets
if [ "$TARGET" = "all" ] || [ "$TARGET" = "assets" ]; then
    echo "[3/3] Packing assets (GameAssets.apak)..."
    if [ -f cmake-build-tools/asset_packer ]; then
        cmake-build-tools/asset_packer -i assets/ -o GameAssets.apak -v --include-lua
    elif [ -f cmake-build-tools/Debug/asset_packer ]; then
        cmake-build-tools/Debug/asset_packer -i assets/ -o GameAssets.apak -v --include-lua
    elif [ -f cmake-build-tools/Release/asset_packer ]; then
        cmake-build-tools/Release/asset_packer -i assets/ -o GameAssets.apak -v --include-lua
    else
        echo "Warning: Asset packer not found, skipping asset packing"
    fi
    echo "✓ GameAssets.apak created"
    echo ""
fi

# Step 4: Build Engine
if [ "$TARGET" = "all" ] || [ "$TARGET" = "engine" ]; then
    echo "[4/4] Building Source67 engine..."
    
    if [ "$BUILD_TYPE" = "Release" ]; then
        cmake -DCMAKE_BUILD_TYPE=Release -DSTANDALONE_MODE=ON -DEDITOR_MODE=OFF -B cmake-build-release
        cmake --build cmake-build-release --config Release
        echo "✓ Source67 built (Standalone mode)"
    else
        cmake -DCMAKE_BUILD_TYPE=Debug -DSTANDALONE_MODE=OFF -DEDITOR_MODE=ON -B cmake-build-debug
        cmake --build cmake-build-debug --config Debug
        echo "✓ Source67 built (Editor mode)"
    fi
    echo ""
fi

echo "========================================="
echo "Build Complete!"
echo "========================================="
echo ""
echo "Distribution package contents:"
if [ "$BUILD_TYPE" = "Release" ]; then
    echo "  - cmake-build-release/Source67 (or Source67.exe)"
    echo "  - GameAssets.apak"
    echo "  - game/build/Release/Game.dll (or libGame.so/dylib)"
    echo ""
    echo "To run: ./cmake-build-release/Source67"
else
    echo "  - cmake-build-debug/Source67 (or Source67.exe)"
    echo "  - GameAssets.apak"
    echo "  - game/build/Debug/Game.dll (or libGame.so/dylib)"
    echo ""
    echo "To run: ./cmake-build-debug/Source67"
fi

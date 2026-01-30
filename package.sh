#!/bin/bash
# ========================================
# Source67 - Distribution Package Script
# ========================================
#
# This script creates a distribution-ready package of your game
# Usage: ./package.sh [GameName] [Version]
# Example: ./package.sh MyGame 1.0.0
#

# Get parameters or use defaults
GAME_NAME="${1:-MyGame}"
VERSION="${2:-1.0.0}"
OUTPUT_DIR="${GAME_NAME}_v${VERSION}"

echo ""
echo "========================================"
echo "Creating Distribution Package"
echo "========================================"
echo "Game: $GAME_NAME"
echo "Version: $VERSION"
echo "Output: $OUTPUT_DIR"
echo ""

# Step 1: Build Release version
echo "[1/7] Building Release version..."
echo "----------------------------------------"
./build.sh Release all
if [ $? -ne 0 ]; then
    echo "ERROR: Build failed!"
    echo "Please fix build errors and try again."
    exit 1
fi
echo "SUCCESS: Release build complete"
echo ""

# Step 2: Create distribution folder
echo "[2/7] Creating distribution folder..."
echo "----------------------------------------"
if [ -d "$OUTPUT_DIR" ]; then
    echo "Removing old package..."
    rm -rf "$OUTPUT_DIR"
fi
mkdir "$OUTPUT_DIR"
echo "SUCCESS: Folder created"
echo ""

# Step 3: Copy game files
echo "[3/7] Copying game files..."
echo "----------------------------------------"

# Copy and rename engine executable
if [ -f "cmake-build-release/Source67" ]; then
    cp "cmake-build-release/Source67" "$OUTPUT_DIR/$GAME_NAME"
    chmod +x "$OUTPUT_DIR/$GAME_NAME"
    echo "Copied: $GAME_NAME"
elif [ -f "cmake-build-release/Release/Source67" ]; then
    cp "cmake-build-release/Release/Source67" "$OUTPUT_DIR/$GAME_NAME"
    chmod +x "$OUTPUT_DIR/$GAME_NAME"
    echo "Copied: $GAME_NAME"
else
    echo "ERROR: Source67 executable not found!"
    echo "Build the engine first with: ./build.sh Release all"
    exit 1
fi

# Detect platform and copy appropriate library
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    LIB_NAME="Game.dylib"
    if [ -f "game/build/Release/libGame.dylib" ]; then
        cp "game/build/Release/libGame.dylib" "$OUTPUT_DIR/$LIB_NAME"
        echo "Copied: $LIB_NAME"
    elif [ -f "game/build/libGame.dylib" ]; then
        cp "game/build/libGame.dylib" "$OUTPUT_DIR/$LIB_NAME"
        echo "Copied: $LIB_NAME"
    else
        echo "ERROR: Game.dylib not found!"
        exit 1
    fi
else
    # Linux
    LIB_NAME="Game.so"
    if [ -f "game/build/Release/libGame.so" ]; then
        cp "game/build/Release/libGame.so" "$OUTPUT_DIR/$LIB_NAME"
        echo "Copied: $LIB_NAME"
    elif [ -f "game/build/libGame.so" ]; then
        cp "game/build/libGame.so" "$OUTPUT_DIR/$LIB_NAME"
        echo "Copied: $LIB_NAME"
    else
        echo "ERROR: Game.so not found!"
        exit 1
    fi
fi

# Copy GameAssets.apak
if [ -f "GameAssets.apak" ]; then
    cp "GameAssets.apak" "$OUTPUT_DIR/GameAssets.apak"
    echo "Copied: GameAssets.apak"
else
    echo "ERROR: GameAssets.apak not found!"
    echo "Build assets first with: ./build.sh Release assets"
    exit 1
fi

echo "SUCCESS: All game files copied"
echo ""

# Step 4: Create launch script
echo "[4/7] Creating launch script..."
echo "----------------------------------------"
cat > "$OUTPUT_DIR/run.sh" << EOF
#!/bin/bash
# Launch script for $GAME_NAME

# Set library path to include current directory
export LD_LIBRARY_PATH=".:$LD_LIBRARY_PATH"

# Run the game
./$GAME_NAME "\$@"
EOF

chmod +x "$OUTPUT_DIR/run.sh"
echo "SUCCESS: run.sh created"
echo ""

# Step 5: Create README
echo "[5/7] Creating README.txt..."
echo "----------------------------------------"
cat > "$OUTPUT_DIR/README.txt" << EOF
========================================
    $GAME_NAME v$VERSION
========================================

HOW TO PLAY:
1. Run: ./run.sh (or double-click if set as executable)
2. Use WASD to move, Mouse to look
3. Press ESC to pause/quit

CONTROLS:
- W/A/S/D: Move
- Mouse: Look around
- Space: Jump
- E: Interact
- ~: Open developer console

SYSTEM REQUIREMENTS:
- Linux or macOS
- OpenGL 4.5 compatible GPU
- 4GB RAM minimum
- 500MB free disk space

VERSION: $VERSION
CREATED WITH: Source67 Game Engine

For support or feedback, visit:
[Your website or contact info here]

Thank you for playing!
========================================
EOF
echo "SUCCESS: README.txt created"
echo ""

# Step 6: Show package info
echo "[6/7] Package contents:"
echo "----------------------------------------"
ls -lh "$OUTPUT_DIR"
echo ""

# Step 7: Create archive
echo "[7/7] Creating archive..."
echo "----------------------------------------"

if [ -f "${OUTPUT_DIR}.tar.gz" ]; then
    rm "${OUTPUT_DIR}.tar.gz"
fi

tar -czf "${OUTPUT_DIR}.tar.gz" "$OUTPUT_DIR"
if [ $? -eq 0 ]; then
    echo "SUCCESS: ${OUTPUT_DIR}.tar.gz created"
    ls -lh "${OUTPUT_DIR}.tar.gz"
else
    echo "WARNING: Archive creation failed"
    echo "Package folder created but not archived"
fi
echo ""

echo "========================================"
echo "Distribution Package Complete!"
echo "========================================"
echo ""
echo "Package folder: $OUTPUT_DIR/"
echo ""
if [ -f "${OUTPUT_DIR}.tar.gz" ]; then
    echo "Archive: ${OUTPUT_DIR}.tar.gz"
    echo ""
    echo "Your game is ready to distribute!"
    echo ""
    echo "Next steps:"
    echo "1. Test the package on a clean system"
    echo "2. Upload to itch.io or your distribution platform"
    echo "3. Share with players!"
else
    echo "Folder created (archive failed)"
    echo "Please manually create an archive from: $OUTPUT_DIR/"
fi
echo ""
echo "For more information, see DISTRIBUTION.md"
echo "========================================"
echo ""

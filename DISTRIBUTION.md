# 📦 Distributing Your Source67 Game

**Complete guide to packaging and publishing your game**

---

## 🎯 The Goal

You've built your game and have:
- ✅ `Game.dll` (your compiled game code)
- ✅ `GameAssets.apak` (your packed assets)

**Now you need:** A complete game package that players can download and run!

---

## 📂 What Files Do You Need?

### The Complete Game Package

```
YourGame/                           ← Distribution folder
├── YourGame.exe                    ← The engine (renamed)
├── Game.dll                        ← Your game code
├── GameAssets.apak                 ← Your assets
├── LICENSE.txt                     ← Your game's license
├── README.txt                      ← How to play
└── (Optional) Additional DLLs      ← System dependencies
```

**That's it!** Just 3 essential files to run your game.

---

## 🔧 Step-by-Step: Creating a Distribution Package

### Quick Method: Use the Package Script ⭐

**Easiest way to create a distribution package:**

```bash
# Windows
package.bat MyGame 1.0.0

# Linux/macOS
./package.sh MyGame 1.0.0
```

**What it does:**
1. Builds Release versions automatically
2. Creates distribution folder
3. Copies and renames all files
4. Creates README.txt
5. Packages everything into a ZIP/archive

**Output:** `MyGame_v1.0.0.zip` ready to distribute!

**Skip to the end if using this method!** ↓

---

### Manual Method: Step-by-Step

**If you prefer to do it manually:**

### Step 1: Build Release Versions

**Important:** Use Release builds, NOT Debug builds!

```bash
# Windows
build.bat Release all

# Linux/macOS
./build.sh Release all
```

**What this does:**
- Compiles with optimizations (faster, smaller)
- Removes debug symbols
- Strips unnecessary code
- Creates production-ready builds

**Output locations:**
```
cmake-build-release/Release/Source67.exe  ← Engine (Release)
game/build/Release/Game.dll               ← Your game (Release)
GameAssets.apak                           ← Your assets (same)
```

### Step 2: Create Distribution Folder

**Create a clean folder for your game:**

```bash
# Windows
mkdir MyGame_v1.0
cd MyGame_v1.0

# Linux/macOS
mkdir MyGame_v1.0
cd MyGame_v1.0
```

### Step 3: Copy Required Files

**Copy the three essential files:**

```bash
# Windows
copy ..\cmake-build-release\Release\Source67.exe MyGame.exe
copy ..\game\build\Release\Game.dll Game.dll
copy ..\GameAssets.apak GameAssets.apak

# Linux/macOS
cp ../cmake-build-release/Source67 MyGame
cp ../game/build/libGame.so Game.so
cp ../GameAssets.apak GameAssets.apak
```

**Important:** Rename `Source67.exe` to your game's name!

### Step 4: Add Game Information Files

**Create README.txt:**
```
===========================================
    MY AWESOME GAME
===========================================

HOW TO PLAY:
1. Double-click MyGame.exe
2. Use WASD to move
3. Mouse to look around
4. ESC to pause

CONTROLS:
- W/A/S/D: Move
- Mouse: Look
- Space: Jump
- E: Interact
- ~: Open console

SYSTEM REQUIREMENTS:
- Windows 10/11 or Linux
- OpenGL 4.5 compatible GPU
- 4GB RAM
- 500MB storage

VERSION: 1.0.0
CREATED BY: Your Name
WEBSITE: yourwebsite.com

Thank you for playing!
===========================================
```

**Create LICENSE.txt:**
```
Your game's license (MIT, GPL, proprietary, etc.)
```

### Step 5: Test Your Package

**Before distributing, test it!**

```
1. Copy the distribution folder to a different location
2. Double-click MyGame.exe
3. Make sure the game runs correctly
4. Test all features
5. Check for missing assets or errors
```

**Common issues:**
- Missing DLLs (see Step 6)
- Wrong file paths
- Assets not loading

### Step 6: Check Dependencies (Windows)

**Some systems might need Visual C++ redistributables:**

```bash
# Check what DLLs your exe needs
dumpbin /dependents MyGame.exe

# Common dependencies:
- vcruntime140.dll
- msvcp140.dll
- ucrtbase.dll
```

**Options:**

**A) Include redistributables installer:**
```
Download from Microsoft:
https://aka.ms/vs/17/release/vc_redist.x64.exe

Include in your package with instructions
```

**B) Static linking (preferred):**
```cmake
# In CMakeLists.txt (already configured)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
```

### Step 7: Package for Distribution

**Create a ZIP archive:**

```bash
# Windows (PowerShell)
Compress-Archive -Path MyGame_v1.0 -DestinationPath MyGame_v1.0.zip

# Linux/macOS
zip -r MyGame_v1.0.zip MyGame_v1.0/

# Or use 7-Zip, WinRAR, etc.
```

**Result:** `MyGame_v1.0.zip` ready to distribute!

---

## 🎮 Platform-Specific Packaging

### Windows Distribution

**Standard Package:**
```
MyGame_v1.0/
├── MyGame.exe               ← Renamed Source67.exe
├── Game.dll
├── GameAssets.apak
├── README.txt
├── LICENSE.txt
└── vc_redist.x64.exe       ← Optional
```

**Installer (Advanced):**
- Use Inno Setup or NSIS
- Creates professional installer
- Adds Start Menu shortcuts
- Handles uninstall

### Linux Distribution

**Package Structure:**
```
MyGame_v1.0/
├── MyGame                   ← Renamed Source67 (executable)
├── libGame.so              ← Game shared library
├── GameAssets.apak
├── README.txt
├── LICENSE.txt
└── run.sh                  ← Launch script
```

**Create run.sh:**
```bash
#!/bin/bash
# Set library path
export LD_LIBRARY_PATH=".:$LD_LIBRARY_PATH"

# Run game
./MyGame "$@"
```

```bash
chmod +x run.sh
chmod +x MyGame
```

**AppImage (Recommended for Linux):**
- Creates single-file executable
- Works on all Linux distributions
- No installation needed

### macOS Distribution

**Package Structure:**
```
MyGame.app/
└── Contents/
    ├── MacOS/
    │   ├── MyGame              ← Executable
    │   ├── Game.dylib          ← Game library
    │   └── GameAssets.apak
    ├── Resources/
    │   └── Icon.icns           ← App icon
    └── Info.plist              ← App metadata
```

**Create .dmg installer:**
- Professional macOS distribution
- Drag-to-install interface
- Code signing for Gatekeeper

---

## 🚀 Publishing on Steam

### Prerequisites

1. **Steamworks Account**
   - Register at partner.steamgames.com
   - Pay the $100 app fee
   - Complete tax forms

2. **Your Game Build**
   - Release build (not Debug!)
   - Tested and working
   - All assets included

### Step 1: Prepare Steam Build

**Create a Steam-specific folder:**
```
MyGame_Steam/
├── MyGame.exe
├── Game.dll
├── GameAssets.apak
├── steam_api64.dll          ← Steamworks SDK
└── steam_appid.txt          ← Your Steam App ID
```

**Add steam_appid.txt:**
```
480                          ← Replace with your actual App ID
```

### Step 2: Integrate Steamworks (Optional)

**For achievements, leaderboards, etc.:**

```cpp
// In game/src/game_api.cpp
#include <steam/steam_api.h>

void game_initialize(void* engine, void* lua) {
    if (!SteamAPI_Init()) {
        // Steam not running or failed
        // Game can still run without Steam features
    }
    
    // Your game init code
}

void game_update(float dt) {
    SteamAPI_RunCallbacks();  // Process Steam events
    
    // Your game update code
}

void game_shutdown() {
    SteamAPI_Shutdown();
}
```

### Step 3: Upload to Steam

**Using SteamCMD or Steamworks GUI:**

1. **Create depot** (your game files)
2. **Create build** (version)
3. **Upload files**
4. **Set launch options**
5. **Test build**

**Depot configuration (depot_build.vdf):**
```
"DepotBuild"
{
    "DepotID" "1001"
    "ContentRoot" "C:\MyGame_Steam\"
    "FileMapping"
    {
        "LocalPath" "*"
        "DepotPath" "."
    }
}
```

**App build script (app_build.vdf):**
```
"AppBuild"
{
    "AppID" "480"
    "Desc" "Build 1.0.0"
    "BuildOutput" "C:\output\"
    "ContentRoot" "C:\MyGame_Steam\"
    "SetLive" "default"
    "Depots"
    {
        "1001" "depot_build.vdf"
    }
}
```

**Upload command:**
```bash
steamcmd +login yourusername +run_app_build app_build.vdf +quit
```

### Step 4: Configure Store Page

1. **Screenshots** (at least 5)
2. **Trailer video**
3. **Description**
4. **System requirements**
5. **Price** (or free)
6. **Release date**

### Step 5: Release!

1. Set build live
2. Publish store page
3. Announce on social media
4. Monitor reviews and feedback

---

## 🎨 Other Distribution Platforms

### itch.io (Easiest!)

**Perfect for indie games:**

1. **Create game page** at itch.io
2. **Upload ZIP file** directly
3. **Set price** (or pay-what-you-want)
4. **Publish!**

**No approval process, instant publishing!**

### GOG

**DRM-free distribution:**

1. Apply for publishing
2. Game review process
3. Galaxy integration (optional)
4. Upload builds

### Epic Games Store

**Higher revenue share (88/12):**

1. Apply for publishing
2. Epic review process
3. Integrate Epic Online Services
4. Upload builds

---

## 📊 Distribution Checklist

### Before You Distribute

- [ ] Build with Release configuration
- [ ] Test on clean system (no dev tools)
- [ ] All assets load correctly
- [ ] No debug logs or console spam
- [ ] Game runs at good FPS
- [ ] No crashes or errors
- [ ] Controls work properly
- [ ] Audio plays correctly
- [ ] All levels accessible
- [ ] Save/load works

### Package Contents

- [ ] Game executable (renamed from Source67.exe)
- [ ] Game.dll (Release build)
- [ ] GameAssets.apak
- [ ] README.txt
- [ ] LICENSE.txt
- [ ] System requirements documented
- [ ] Version number in filename

### Optional But Recommended

- [ ] Game icon
- [ ] Installer (Windows)
- [ ] Desktop shortcuts
- [ ] Uninstaller
- [ ] Auto-update system
- [ ] Analytics/crash reporting
- [ ] Achievements (if using Steam/Epic)

---

## 🛠️ Automated Packaging Script

### Windows (package.bat)

```batch
@echo off
echo ========================================
echo Packaging Game for Distribution
echo ========================================

set GAME_NAME=MyGame
set VERSION=1.0.0
set OUTPUT_DIR=%GAME_NAME%_v%VERSION%

echo Step 1: Building Release version...
call build.bat Release all
if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

echo Step 2: Creating distribution folder...
if exist %OUTPUT_DIR% rmdir /s /q %OUTPUT_DIR%
mkdir %OUTPUT_DIR%

echo Step 3: Copying files...
copy cmake-build-release\Release\Source67.exe %OUTPUT_DIR%\%GAME_NAME%.exe
copy game\build\Release\Game.dll %OUTPUT_DIR%\Game.dll
copy GameAssets.apak %OUTPUT_DIR%\GameAssets.apak

echo Step 4: Creating README...
(
echo ========================================
echo     %GAME_NAME% v%VERSION%
echo ========================================
echo.
echo Double-click %GAME_NAME%.exe to play!
echo.
echo Controls: See in-game help
echo.
echo Enjoy!
echo ========================================
) > %OUTPUT_DIR%\README.txt

echo Step 5: Creating ZIP...
powershell Compress-Archive -Path %OUTPUT_DIR% -DestinationPath %OUTPUT_DIR%.zip -Force

echo ========================================
echo Package created: %OUTPUT_DIR%.zip
echo ========================================
pause
```

### Linux/macOS (package.sh)

```bash
#!/bin/bash

GAME_NAME="MyGame"
VERSION="1.0.0"
OUTPUT_DIR="${GAME_NAME}_v${VERSION}"

echo "========================================"
echo "Packaging Game for Distribution"
echo "========================================"

echo "Step 1: Building Release version..."
./build.sh Release all
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Step 2: Creating distribution folder..."
rm -rf "$OUTPUT_DIR"
mkdir "$OUTPUT_DIR"

echo "Step 3: Copying files..."
cp cmake-build-release/Source67 "$OUTPUT_DIR/$GAME_NAME"
cp game/build/libGame.so "$OUTPUT_DIR/Game.so"
cp GameAssets.apak "$OUTPUT_DIR/GameAssets.apak"
chmod +x "$OUTPUT_DIR/$GAME_NAME"

echo "Step 4: Creating launch script..."
cat > "$OUTPUT_DIR/run.sh" << 'EOF'
#!/bin/bash
export LD_LIBRARY_PATH=".:$LD_LIBRARY_PATH"
./$GAME_NAME "$@"
EOF
chmod +x "$OUTPUT_DIR/run.sh"

echo "Step 5: Creating README..."
cat > "$OUTPUT_DIR/README.txt" << EOF
========================================
    $GAME_NAME v$VERSION
========================================

To run: ./run.sh

Controls: See in-game help

Enjoy!
========================================
EOF

echo "Step 6: Creating archive..."
tar -czf "${OUTPUT_DIR}.tar.gz" "$OUTPUT_DIR"

echo "========================================"
echo "Package created: ${OUTPUT_DIR}.tar.gz"
echo "========================================"
```

---

## 🎯 Quick Reference

### Minimum Distribution Package

```
Game Files:
  1. YourGame.exe (or executable)    ← Renamed Source67.exe
  2. Game.dll (or .so/.dylib)        ← Your game code
  3. GameAssets.apak                 ← Your assets

Documentation:
  4. README.txt                      ← How to play
  5. LICENSE.txt                     ← Legal stuff
```

### Build Commands

```bash
# Release build (always use for distribution!)
Windows: build.bat Release all
Linux:   ./build.sh Release all

# Output locations
Engine:  cmake-build-release/Release/Source67.exe
Game:    game/build/Release/Game.dll
Assets:  GameAssets.apak (root folder)
```

### File Sizes (Approximate)

```
Source67.exe:     ~15-30 MB
Game.dll:         ~1-5 MB (depends on your code)
GameAssets.apak:  Varies by content (could be 100MB+)

Total:            Usually 20-100 MB
```

---

## ⚠️ Important Notes

### Do NOT Include

- ❌ Debug builds (slow, large, unstable)
- ❌ .pdb files (debug symbols)
- ❌ Source code
- ❌ Build folders (cmake-build-*)
- ❌ .git folder
- ❌ Development tools

### DO Include

- ✅ Release builds only
- ✅ All required DLLs
- ✅ README with instructions
- ✅ License file
- ✅ System requirements
- ✅ Your contact info/support

### Testing Distribution

**Always test on a different computer!**

Ideally:
- Clean Windows install (no Visual Studio)
- Different user account
- Virtual machine

This ensures you haven't missed dependencies.

---

## 🚢 Distribution Workflow

```
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│  1. DEVELOP                                                 │
│     ├─ Write code                                           │
│     ├─ Create assets                                        │
│     └─ Test in editor                                       │
│                                                             │
│  2. BUILD RELEASE                                           │
│     ├─ build.bat Release all                                │
│     ├─ Test release build                                   │
│     └─ Fix any release-specific issues                      │
│                                                             │
│  3. PACKAGE                                                 │
│     ├─ Create distribution folder                           │
│     ├─ Copy: YourGame.exe, Game.dll, GameAssets.apak       │
│     ├─ Add: README, LICENSE                                 │
│     └─ Create ZIP/installer                                 │
│                                                             │
│  4. TEST PACKAGE                                            │
│     ├─ Extract on clean system                              │
│     ├─ Run game                                             │
│     ├─ Check all features                                   │
│     └─ Fix any issues                                       │
│                                                             │
│  5. DISTRIBUTE                                              │
│     ├─ Upload to Steam/itch.io/etc.                         │
│     ├─ Create store page                                    │
│     ├─ Market your game                                     │
│     └─ Support players                                      │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## 🎓 Summary

### To create a distributable game:

1. **Build Release version:**
   ```bash
   build.bat Release all
   ```

2. **Collect 3 files:**
   - Source67.exe → Rename to YourGame.exe
   - Game.dll
   - GameAssets.apak

3. **Add documentation:**
   - README.txt
   - LICENSE.txt

4. **Package:**
   - Create folder
   - Copy files
   - ZIP it up

5. **Test and distribute!**

**That's it!** Your game is ready to share with the world! 🎮

---

## 📚 Related Documentation

- **BUILDING.md** - How to build your game
- **GAME_PROJECT_GUIDE.md** - Understanding the architecture
- **START_HERE_NEW_USER.md** - Getting started

---

**Ready to publish your game? Follow this guide and you'll have a professional distribution package!** 🚀

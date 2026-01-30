# 🚀 Source67 Quick Start Guide

**Welcome to Source67!** This guide will walk you through building and running the engine for the first time.

---

## 📋 Table of Contents

1. [Prerequisites](#prerequisites)
2. [First Time Setup](#first-time-setup)
3. [Building the Engine](#building-the-engine)
4. [Running the Engine](#running-the-engine)
5. [What to Expect](#what-to-expect)
6. [Troubleshooting](#troubleshooting)
7. [Next Steps](#next-steps)

---

## ✅ Prerequisites

Before you start, make sure you have:

### Windows
- **CMake** 3.20 or higher ([Download](https://cmake.org/download/))
- **Visual Studio 2019/2022** with C++ Desktop Development workload
  - Or **MinGW-w64** / **MSYS2** with GCC
- **Git** (for cloning the repository)
- **Windows 10/11**

### Linux
- **CMake** 3.20+: `sudo apt install cmake` (Ubuntu/Debian)
- **GCC/G++** with C++20 support: `sudo apt install build-essential`
- **OpenGL development libraries**: `sudo apt install libgl1-mesa-dev`
- **X11 development libraries**: `sudo apt install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev`

### macOS
- **Xcode** Command Line Tools: `xcode-select --install`
- **CMake**: `brew install cmake`
- **OpenGL** (included with macOS)

---

## 🎯 First Time Setup

### Step 1: Clone the Repository

```bash
# Navigate to where you want the project
cd C:\Users\YourName\Projects  # Windows
cd ~/Projects                   # Linux/macOS

# Clone the repository
git clone https://github.com/JsemOlik/Source67.git
cd Source67
```

### Step 2: Verify the File Structure

You should see these important files in the root directory:

```
Source67/
├── build.bat          ← Windows build script
├── build.sh           ← Linux/macOS build script
├── CMakeLists.txt     ← Main build configuration
├── src/               ← Engine source code
├── game/              ← Your game code goes here
├── assets/            ← Game assets (models, textures, scripts)
└── tools/             ← Build tools (asset packer)
```

**✅ You are now in the right place to run the build scripts!**

---

## 🔨 Building the Engine

The Source67 engine uses a **hybrid build system** that compiles:
1. **Game.dll** - Your C++ game code
2. **GameAssets.apak** - Packed game assets
3. **Source67.exe** - The main engine executable

### Windows: Using build.bat

#### Method 1: Command Prompt (Recommended)

1. **Open Command Prompt** (not PowerShell initially)
   - Press `Win + R`
   - Type `cmd` and press Enter

2. **Navigate to the Source67 folder**
   ```cmd
   cd C:\Path\To\Source67
   ```

3. **Run the build script**
   ```cmd
   build.bat Debug all
   ```

   **What do these parameters mean?**
   - `Debug` - Build type (use `Debug` for development, `Release` for final builds)
   - `all` - Build everything (game DLL, asset pack, and engine)

#### Method 2: Double-Click (With Modifications)

If you want to double-click `build.bat`, you'll need to modify it slightly (see [Improving build.bat](#improving-buildbat) section below).

#### What Happens When You Run build.bat?

The script will:

```
[1/4] Building Game.dll...
  └─ Compiles C++ game code from the game/ folder
  └─ Creates: game/build/Debug/Game.dll

[2/4] Building asset packer tool...
  └─ Compiles the asset packer utility
  └─ Creates: cmake-build-tools/Debug/asset_packer.exe

[3/4] Packing assets (GameAssets.apak)...
  └─ Packs all files from assets/ folder
  └─ Includes Lua scripts from assets/lua/
  └─ Creates: GameAssets.apak (in root folder)

[4/4] Building Source67 engine...
  └─ Compiles the main engine from src/ folder
  └─ Creates: cmake-build-debug/Debug/Source67.exe
```

**⏱️ This will take 2-5 minutes on the first build** (dependencies need to download and compile).

### Linux/macOS: Using build.sh

1. **Open Terminal**

2. **Navigate to the Source67 folder**
   ```bash
   cd ~/Path/To/Source67
   ```

3. **Make the script executable** (first time only)
   ```bash
   chmod +x build.sh
   ```

4. **Run the build script**
   ```bash
   ./build.sh Debug all
   ```

---

## 🎮 Running the Engine

### After Successful Build

Once the build completes, you'll see this message:

```
=========================================
Build Complete!
=========================================

Distribution package contents:
  - cmake-build-debug\Debug\Source67.exe
  - GameAssets.apak
  - game\build\Debug\Game.dll

To run: cmake-build-debug\Debug\Source67.exe
```

### Windows

**Method 1: From Command Prompt**
```cmd
cmake-build-debug\Debug\Source67.exe
```

**Method 2: Double-Click**
Navigate to `cmake-build-debug\Debug\` and double-click `Source67.exe`

**Method 3: Create a Shortcut**
Right-click `Source67.exe` → Send to → Desktop (create shortcut)

### Linux/macOS

```bash
./cmake-build-debug/Source67
```

---

## 🎨 What to Expect

When the engine launches, you should see:

### Editor Mode (Debug Build)

![Source67 Editor](docs/editor-screenshot.png)

**Main Window with:**
- **Scene Viewport** (left) - 3D view of your scene
- **Game Viewport** (right) - Game camera view
- **Scene Hierarchy** (left panel) - List of entities
- **Inspector** (right panel) - Entity properties
- **Content Browser** (bottom) - Asset browser
- **Toolbar** (top) - Play/Pause/Stop controls
- **Console** (toggleable with `~` key)

### Console Output

In the terminal/console, you should see:

```
[Core] Initializing Window...
[Core] Initializing Renderer...
[Core] Initializing Physics...
[Core] Initializing Lua Engine...
[Core] Initializing Hybrid Build System...
[Core] Loading asset pack: C:/Path/To/GameAssets.apak
[Core] Asset pack loaded successfully!
[Core]   Total assets: 17
[Core]   Lua scripts: 7
[Core] Loading Game DLL: C:/Path/To/game/build/Debug/Game.dll
[Core] Game DLL loaded successfully!
[Game DLL] Initialized!
[Core] Application initialized successfully
```

**✅ If you see this, everything is working perfectly!**

### If DLL/Assets Are Missing

```
[Core] Initializing Hybrid Build System...
[Core] Hybrid build system not available (DLL/assets not found - this is normal if not built yet)
```

**This is fine!** It means the engine is running, but the optional game DLL and asset pack weren't found. The engine still works normally.

---

## ⚠️ Troubleshooting

### Problem: "The build window closes immediately"

**Cause:** The script runs so fast you can't see output, or there's an error.

**Solution 1:** Run from Command Prompt (see [Method 1](#method-1-command-prompt-recommended))

**Solution 2:** Use the improved build.bat (see below)

### Problem: "CMake not found" or "cmake is not recognized"

**Cause:** CMake is not installed or not in your PATH.

**Solution:**
1. Download CMake from https://cmake.org/download/
2. During installation, select "Add CMake to system PATH"
3. Restart Command Prompt and try again

### Problem: "No compiler found" or "Visual Studio not found"

**Cause:** No C++ compiler is installed.

**Solution (Windows):**
1. Install Visual Studio 2022 Community (free)
2. During installation, select "Desktop development with C++"
3. Restart and try again

**Alternative:** Install MinGW-w64 via MSYS2

### Problem: Build fails with "X11 not found" (Linux)

**Cause:** Missing development libraries.

**Solution:**
```bash
sudo apt install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
```

### Problem: "Game.dll not found" when running

**Cause:** The game DLL wasn't built successfully.

**Solution:**
```cmd
# Build just the game DLL
build.bat Debug game

# Check if it was created
dir game\build\Debug\Game.dll
```

### Problem: "GameAssets.apak not found"

**Cause:** Assets weren't packed.

**Solution:**
```cmd
# Build just the assets
build.bat Debug assets

# Check if it was created
dir GameAssets.apak
```

### Problem: Engine crashes on startup

**Check:**
1. Make sure you're running from the Source67 root directory
2. Check that `assets/` folder exists
3. Look at console output for specific error messages

**Common fix:**
```cmd
# Rebuild everything from scratch
rmdir /s /q cmake-build-debug
rmdir /s /q cmake-build-tools
rmdir /s /q game\build
build.bat Debug all
```

---

## 🎓 Next Steps

Now that the engine is running, here's what you can do:

### 1. Explore the Editor

- **Play with the Scene Viewport**: Right-click + drag to rotate camera
- **Select Entities**: Click objects in the Scene Hierarchy
- **Move Objects**: Use the gizmo (press `W` for translate, `E` for rotate, `R` for scale)
- **Open Console**: Press `~` key, type `help` to see commands

### 2. Create Your First Scene

1. **File → New Scene**
2. **Right-click in Scene Hierarchy → Create Entity**
3. **Select the entity and modify properties in Inspector**
4. **File → Save Scene** (saves as `.s67` file in `assets/scenes/`)

### 3. Write Game Code

Edit files in the `game/src/` directory:

```cpp
// game/src/MyGameScript.cpp
#include "../../../src/Renderer/ScriptableEntity.h"
#include "../../../src/Renderer/ScriptRegistry.h"

class MyGameScript : public S67::ScriptableEntity {
public:
    void OnCreate() override {
        S67_INFO("My game started!");
    }
    
    void OnUpdate(float deltaTime) override {
        // Your game logic here
    }
};

REGISTER_SCRIPT(MyGameScript);
```

**Then rebuild:**
```cmd
build.bat Debug game
```

### 4. Add Assets

Place your assets in the `assets/` folder:
- **Models**: `assets/models/` (`.obj` files)
- **Textures**: `assets/textures/` (`.png`, `.jpg`)
- **Shaders**: `assets/shaders/` (`.glsl`)
- **Lua Scripts**: `assets/lua/` (`.lua`)

**Then repack:**
```cmd
build.bat Debug assets
```

### 5. Learn More

- **Scripting**: Read `README.md` → Scripting Guide section
- **Build System**: Read `BUILD_SYSTEM_README.md` for advanced options
- **Engine Architecture**: Read `SYSTEM_ARCHITECTURE.md`
- **API Documentation**: Read `CODING-API.md`

---

## 📝 Build Script Quick Reference

### Build Types

| Command | What it builds | Use when |
|---------|----------------|----------|
| `build.bat Debug all` | Everything (Debug) | First time, or developing |
| `build.bat Release all` | Everything (Release) | Making final build |
| `build.bat Debug game` | Just Game.dll | Changed game code |
| `build.bat Debug assets` | Just GameAssets.apak | Added/changed assets |
| `build.bat Debug engine` | Just Source67.exe | Changed engine code |

### Common Workflows

**Daily Development:**
```cmd
# 1. Make changes to game code
# Edit files in game/src/

# 2. Rebuild game DLL
build.bat Debug game

# 3. Run engine
cmake-build-debug\Debug\Source67.exe
```

**Adding New Assets:**
```cmd
# 1. Add files to assets/ folder
# 2. Repack assets
build.bat Debug assets

# 3. Run engine
cmake-build-debug\Debug\Source67.exe
```

**Clean Rebuild:**
```cmd
# Delete all build folders
rmdir /s /q cmake-build-debug
rmdir /s /q game\build

# Rebuild everything
build.bat Debug all
```

---

## 🎯 Summary

**To build and run Source67 for the first time:**

1. Open **Command Prompt**
2. Navigate to Source67 folder: `cd C:\Path\To\Source67`
3. Run: `build.bat Debug all`
4. Wait 2-5 minutes
5. Run: `cmake-build-debug\Debug\Source67.exe`
6. **Done!** You should see the editor window.

**Where to run build.bat from:**
- **Always from the Source67 root directory** (where the build.bat file is located)
- **Not** from inside subfolders like `game/`, `src/`, or `build/`

**The build.bat file location:**
- It's already in the repository root
- You don't need to move it or copy it anywhere
- You don't "make your game with the engine then place build.bat" - it's already there!

---

## 💡 Tips

- **Use Debug builds** while developing (faster compile times, better error messages)
- **Use Release builds** for final distribution (optimized, faster runtime)
- **Run from Command Prompt** to see build output and catch errors
- **The first build is slow** (5-10 min) because dependencies download. Subsequent builds are much faster (30 sec - 2 min)
- **Keep the console window open** while the engine is running to see log messages

---

## 📞 Getting Help

If you run into issues:

1. **Check the console output** for error messages
2. **Read the Troubleshooting section** above
3. **Check existing documentation**:
   - `BUILD_SYSTEM_README.md` - Build system details
   - `INTEGRATION_SUCCESS.md` - Integration guide
   - `README.md` - General engine overview
4. **File an issue** on GitHub with:
   - Your OS and compiler version
   - The exact command you ran
   - The complete error message
   - What you expected vs. what happened

---

**Happy Game Development!** 🎮✨

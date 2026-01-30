# 🎯 Native Build System - Complete Guide

**The build system is now integrated directly into the editor with native C++ code!**

---

## ✅ What Changed

### Before (Script-Based)
```
Editor → build.bat/build.sh → CMake → Outputs in engine root
                             ↓
                    Hard to find outputs
                    No separate project support
                    Platform-specific scripts
```

### After (Native C++)
```
Editor → BuildSystem (C++) → CMake → Outputs in PROJECT/build/
                           ↓
                  Cross-platform
                  Project-aware
                  Progress reporting
```

---

## 📂 Build Output Location

### For Integrated Mode (Source67 Repository)
```
Source67/
├── game/                    ← Your game code
├── assets/                  ← Your assets
└── build/                   ← NEW! Build outputs here
    ├── Debug/
    │   └── Game.dll         ← Your compiled game
    └── GameAssets.apak      ← Your packed assets
```

### For Separate Projects (Your Portal Example)
```
C:\Users\olik\Desktop\Portal/
├── Portal.source            ← Project manifest
├── game/                    ← Your C++ game code
├── assets/                  ← Your assets (textures, models, Lua)
├── scenes/                  ← Your .s67 scene files
└── build/                   ← NEW! Build outputs here
    ├── Debug/
    │   └── Game.dll         ← Compiled from game/
    ├── GameAssets.apak      ← Packed from assets/
    └── Portal_v1.0.0/       ← Package folder
        ├── Portal.exe       ← Renamed Source67.exe
        ├── Game.dll
        ├── GameAssets.apak
        └── README.txt
```

---

## 🎮 How to Use It

### Step 1: Open Your Project

**Option A: Integrated Mode (for learning)**
- Just open Source67.exe
- The game/ and assets/ folders are in the engine repository
- Build outputs go to `Source67/build/`

**Option B: Separate Project (recommended for real games)**
1. Create a project folder (e.g., `C:\Users\olik\Desktop\Portal`)
2. Create these folders inside:
   ```
   Portal/
   ├── game/          ← Your C++ code
   │   └── src/
   ├── assets/        ← Your assets
   └── scenes/        ← Your scenes
   ```
3. In Source67 editor: **File > Open Project**
4. Select the Portal folder
5. Build outputs go to `Portal/build/`

### Step 2: Build Your Game

**From the Editor:**
```
Building > Build Game (F7)
  - Compiles game/ → Game.dll
  - Output: build/Debug/Game.dll

Building > Build Assets
  - Packs assets/ → GameAssets.apak
  - Output: build/GameAssets.apak

Building > Build All (Ctrl+F7)
  - Builds both Game.dll and GameAssets.apak

Building > Package for Distribution (Ctrl+Shift+F7)
  - Builds everything in Release mode
  - Creates distribution package folder
  - Output: build/ProjectName_v1.0.0/
```

### Step 3: Find Your Outputs

**Console shows exactly where files are:**
```
[BuildSystem] ========================================
[BuildSystem] Game.dll built successfully!
[BuildSystem] Output: C:\Users\olik\Desktop\Portal\build\Debug\Game.dll
[BuildSystem] ========================================
```

**Open the folder:**
```
Building > Open Build Folder
  - Opens file explorer at the engine root
  - Navigate to your project/build/ folder
  - Or just look at the console output paths
```

---

## 🔧 How It Works (Technical)

### The Native BuildSystem

**Instead of calling batch scripts, the editor now:**

1. **Configures itself**
   ```cpp
   BuildConfig config;
   config.projectRoot = "C:\\Users\\olik\\Desktop\\Portal";
   config.engineRoot = "C:\\Users\\olik\\Desktop\\Coding\\Source67";
   config.buildOutputDir = "C:\\Users\\olik\\Desktop\\Portal\\build";
   ```

2. **Builds Game.dll natively**
   ```cpp
   cmake -DCMAKE_BUILD_TYPE=Debug 
         -B "C:\...\Portal\build" 
         -S "C:\...\Portal\game"
   
   cmake --build "C:\...\Portal\build" --config Debug
   ```

3. **Packs assets natively**
   ```cpp
   AssetPacker packer;
   packer.PackAssets("C:\\...\\Portal\\assets", 
                     "C:\\...\\Portal\\build\\GameAssets.apak");
   ```

**Benefits:**
- ✅ Cross-platform (same code on Windows/Linux/macOS)
- ✅ No external scripts needed
- ✅ Progress reporting in console
- ✅ Outputs always in project's build/ folder
- ✅ Works with separate projects

---

## 📝 Project Structure

### Integrated Mode (Simple)
```
Source67/                              ← Clone the engine repo
├── src/                               ← Engine source
├── game/                              ← Your game C++ code
├── assets/                            ← Your game assets
├── build/                             ← Build outputs (auto-created)
│   ├── Debug/Game.dll
│   └── GameAssets.apak
└── Source67.exe                       ← The engine
```

**When you build:**
- Opens: Source67.exe
- Builds: `game/` → `build/Debug/Game.dll`
- Packs: `assets/` → `build/GameAssets.apak`

### Separate Project Mode (Recommended)
```
C:\Users\olik\Desktop\
├── Source67/                          ← Engine (one copy)
│   ├── Source67.exe
│   └── ...
│
└── Portal/                            ← Your game project
    ├── Portal.source                  ← Project manifest
    ├── game/                          ← Your game code
    │   ├── CMakeLists.txt
    │   └── src/
    │       ├── portal_gun.cpp
    │       ├── player.cpp
    │       └── puzzle.cpp
    ├── assets/                        ← Your assets
    │   ├── textures/
    │   │   ├── portal_blue.png
    │   │   └── portal_orange.png
    │   ├── models/
    │   │   └── cube.obj
    │   └── lua/
    │       └── button.lua
    ├── scenes/                        ← Your scenes
    │   └── test_chamber.s67
    └── build/                         ← Build outputs (auto-created)
        ├── Debug/
        │   └── Game.dll               ← Compiled from game/
        ├── GameAssets.apak            ← Packed from assets/
        └── Portal_v1.0.0/             ← Package (if you package)
```

**When you build:**
- Opens: Source67.exe with Portal project loaded
- Builds: `Portal/game/` → `Portal/build/Debug/Game.dll`
- Packs: `Portal/assets/` → `Portal/build/GameAssets.apak`

---

## 🎯 Answering Your Questions

### Q: "Where do I find GameAssets.apak?"

**A:** In your project's `build/` folder!

**For integrated mode:**
```
Source67/build/GameAssets.apak
```

**For separate project:**
```
C:\Users\olik\Desktop\Portal\build\GameAssets.apak
```

The console tells you exactly:
```
[BuildSystem] Output: C:\Users\olik\Desktop\Portal\build\GameAssets.apak
```

### Q: "Is it using assets from my project folder?"

**A:** YES! The BuildSystem automatically detects:
- If you have a project open → uses that project's `assets/` folder
- If no project open → uses engine's `assets/` folder (integrated mode)

### Q: "How do I use this with my Portal project?"

**Step by step:**

1. **Organize your Portal folder:**
   ```
   C:\Users\olik\Desktop\Portal\
   ├── game/
   │   ├── CMakeLists.txt  (copy from Source67/game/)
   │   └── src/
   │       └── game_api.cpp
   ├── assets/
   │   ├── textures/
   │   ├── models/
   │   └── lua/
   ```

2. **Open in Source67:**
   - Run `Source67.exe`
   - **File > Open Project**
   - Select `C:\Users\olik\Desktop\Portal`

3. **Build:**
   - **Building > Build All** (Ctrl+F7)
   - Outputs go to `Portal/build/`

4. **Find your files:**
   - `Portal/build/Debug/Game.dll`
   - `Portal/build/GameAssets.apak`

### Q: "Can we integrate building into the editor itself?"

**A:** DONE! ✅

The editor now has native C++ build logic. No more batch/bash scripts needed. Everything is handled by the `BuildSystem` class in C++.

---

## 🚀 What This Means

### Old Way (Scripts)
```
1. Click "Build Game" in editor
2. Editor calls build.bat
3. build.bat runs in engine directory
4. Outputs scattered in engine repo
5. Hard to find
6. Doesn't work with separate projects
```

### New Way (Native)
```
1. Click "Build Game" in editor
2. Editor uses BuildSystem (C++)
3. BuildSystem detects your project location
4. Creates build/ folder in your project
5. Outputs clearly shown in console
6. Works perfectly with separate projects
```

---

## 📊 Console Output Example

```
[Core] Building Game.dll with native build system...
[BuildSystem] ========================================
[BuildSystem] Building Game.dll...
[BuildSystem] ========================================
[BuildSystem] Configuring CMake for Game.dll...
[BuildSystem] Executing: cmake -DCMAKE_BUILD_TYPE=Debug -B "C:\Users\olik\Desktop\Portal\build" -S "C:\Users\olik\Desktop\Portal\game"
[BuildSystem] Working directory: C:\Users\olik\Desktop\Portal\game
[BuildSystem] Compiling Game.dll...
[BuildSystem] Executing: cmake --build "C:\Users\olik\Desktop\Portal\build" --config Debug
[BuildSystem] Working directory: C:\Users\olik\Desktop\Portal
[BuildSystem] ========================================
[BuildSystem] Game.dll built successfully!
[BuildSystem] Output: C:\Users\olik\Desktop\Portal\build\Debug\Game.dll
[BuildSystem] ========================================
[Core] ==========================================
[Core] Game.dll built successfully!
[Core] Output: C:\Users\olik\Desktop\Portal\build\Debug\Game.dll
[Core] ==========================================
```

**Clear. Precise. Project-aware!**

---

## ✅ Summary

**What you get:**
1. ✅ Build system integrated into editor (no external scripts)
2. ✅ Outputs go to `PROJECT/build/` folder
3. ✅ Works with separate project folders
4. ✅ Clear console output showing file locations
5. ✅ Cross-platform (Windows, Linux, macOS)
6. ✅ Progress reporting
7. ✅ Automatic build folder creation

**How to use:**
1. Open your project in Source67 editor
2. Click **Building > Build Game** or **Build All**
3. Check console for output paths
4. Find files in `YourProject/build/`

**No more confusion about where files are or which assets are being used!** 🎉

# 🔨 Building Your Source67 Game

**Quick, visual guide to building your game with Source67**

---

## 🎯 Three Ways to Build

### 1. In-Editor (Easiest!) ⭐

```
┌─────────────────────────────────────────────────────────┐
│  Source67 Editor                                        │
│  ┌────────────────────────────────────────────────────┐ │
│  │  File  Edit  Settings  Building  Window           │ │
│  │                        ▼                           │ │
│  │                    ┌─────────────────────┐         │ │
│  │                    │ Build Game      F7  │         │ │
│  │                    │ Build Assets        │         │ │
│  │                    │ Build All    Ctrl+F7│         │ │
│  │                    │ ───────────────────│         │ │
│  │                    │ Open Build Folder   │         │ │
│  │                    └─────────────────────┘         │ │
│  └────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘

Just click Building > Build Game (or press F7)!
The editor handles everything for you.
```

**When to use:**
- ✅ Daily development
- ✅ Quick iterations
- ✅ Testing changes
- ✅ You have the editor open anyway

### 2. Command Line (Power Users)

```bash
# Build just the game code (C++)
build.bat Debug game         # Windows
./build.sh Debug game        # Linux/macOS

# Build just assets (textures, Lua, models)
build.bat Debug assets       # Windows
./build.sh Debug assets      # Linux/macOS

# Build everything (game + assets + engine)
build.bat Debug all          # Windows
./build.sh Debug all         # Linux/macOS
```

**When to use:**
- ✅ Automated build scripts
- ✅ CI/CD pipelines
- ✅ Batch operations
- ✅ Editor isn't running

### 3. Visual Studio / Your IDE

Set up build tasks in your IDE:
- Configure CMake
- Add build targets
- Use IDE's build button

**When to use:**
- ✅ Deep C++ debugging
- ✅ Code refactoring
- ✅ Advanced development

---

## 🏗️ What Gets Built?

### The Build Process Visualized

```
┌────────────────────────────────────────────────────────────────┐
│                                                                │
│  YOU CLICK: Building > Build Game                             │
│                                                                │
│  ┌──────────────────┐                                         │
│  │  game/src/       │                                         │
│  │  ├─ player.cpp   │                                         │
│  │  ├─ enemy.cpp    │  ──→  [C++ Compiler]  ──→  Game.dll    │
│  │  └─ gun.cpp      │         (30 seconds)                    │
│  └──────────────────┘                                         │
│                                                                │
│  ✓ Game.dll created in game/build/Debug/                     │
│  ✓ Your C++ code is now ready to run!                        │
│                                                                │
└────────────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────────────┐
│                                                                │
│  YOU CLICK: Building > Build Assets                           │
│                                                                │
│  ┌──────────────────┐                                         │
│  │  assets/         │                                         │
│  │  ├─ textures/    │                                         │
│  │  ├─ models/      │  ──→  [Asset Packer]  ──→              │
│  │  ├─ scenes/      │         (10 seconds)                    │
│  │  └─ lua/         │                      GameAssets.apak    │
│  └──────────────────┘                                         │
│                                                                │
│  ✓ GameAssets.apak created in project root                   │
│  ✓ All assets packed into one file!                          │
│                                                                │
└────────────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────────────┐
│                                                                │
│  AT RUNTIME: Source67.exe loads both                          │
│                                                                │
│  Source67.exe                                                 │
│      │                                                         │
│      ├──→ Loads Game.dll         (your C++ game code)        │
│      │      └─→ Calls game_initialize()                      │
│      │          Calls game_update() every frame              │
│      │                                                         │
│      └──→ Loads GameAssets.apak  (your assets + Lua)         │
│             └─→ Extracts textures, models                     │
│                 Executes Lua scripts                          │
│                 Loads scenes                                  │
│                                                                │
│  ✓ Your complete game is running!                            │
│                                                                │
└────────────────────────────────────────────────────────────────┘
```

---

## 📝 Step-by-Step Tutorial

### Tutorial 1: Building Your First Game

**Step 1: Open Source67**
```
1. Double-click Source67.exe
2. File > Open Project... (or create new)
3. Load a scene (or create one)
```

**Step 2: Make Changes**
```
Option A - Edit C++ code:
  1. Edit files in game/src/
  2. Example: game/src/game_api.cpp

Option B - Edit Lua scripts:
  1. Edit files in assets/lua/
  2. Example: assets/lua/player.lua

Option C - Add assets:
  1. Add textures to assets/textures/
  2. Add models to assets/models/
```

**Step 3: Build**
```
In the editor:
  1. Click "Building" in the menu bar
  2. Choose what to build:
     - "Build Game" (F7) → if you changed C++
     - "Build Assets" → if you changed assets/Lua
     - "Build All" (Ctrl+F7) → everything
  3. Watch the console for build output
```

**Step 4: Test**
```
Option A - Restart engine:
  1. Close Source67.exe
  2. Reopen it
  3. Your changes are loaded!

Option B - Hot reload (Lua only):
  1. File > Reload Scene
  2. Lua changes apply instantly!
```

### Tutorial 2: Daily Development Workflow

**Scenario: Working on Your Portal Game**

**Morning - Start Work:**
```
1. Open Source67.exe
2. File > Open Project > Desktop/Portal/
3. File > Open Level > test_chamber.s67
```

**Adding New Portal Gun Feature (C++):**
```
1. Edit game/src/portal_gun.cpp
   → Add new function ShootOrangePortal()
   
2. Building > Build Game (or press F7)
   → Wait 30 seconds
   → Console shows: "Game.dll built successfully!"
   
3. Close and reopen Source67.exe
   → Test your new feature!
```

**Tweaking Door Behavior (Lua):**
```
1. Edit assets/lua/door.lua
   → Change openSpeed from 2.0 to 5.0
   
2. Building > Build Assets
   → Wait 10 seconds
   → Console shows: "GameAssets.apak built successfully!"
   
3. File > Reload Scene (or F5)
   → Door opens faster instantly!
```

**Adding New Texture:**
```
1. Copy portal_wall.png to assets/textures/
2. Building > Build Assets
3. Reload scene
4. Apply texture in editor
```

**End of Day - Final Build:**
```
1. File > Save Level (Ctrl+S)
2. Building > Build All (Ctrl+F7)
   → Builds everything fresh
3. Test the game one last time
4. Close editor
```

### Tutorial 3: Preparing for Distribution

**Step 1: Clean Build**
```
1. Delete old builds:
   - game/build/
   - cmake-build-debug/
   - GameAssets.apak

2. Building > Build All
   → Fresh, clean build
```

**Step 2: Release Build**
```
From command line:
  Windows: build.bat Release all
  Linux:   ./build.sh Release all

This creates optimized versions:
  - game/build/Release/Game.dll
  - GameAssets.apak
  - cmake-build-release/Source67.exe
```

**Step 3: Package Your Game**
```
Create a folder with:
  Portal/
  ├── Source67.exe          (from cmake-build-release/)
  ├── Game.dll              (from game/build/Release/)
  └── GameAssets.apak       (from root)

Zip it up → Ready to share!
```

---

## ⚡ Quick Reference

### Build Shortcuts

| Action | Menu | Shortcut | Time |
|--------|------|----------|------|
| Build game C++ code | Building > Build Game | F7 | ~30 sec |
| Build assets & Lua | Building > Build Assets | - | ~10 sec |
| Build everything | Building > Build All | Ctrl+F7 | ~2-5 min |
| Open build folder | Building > Open Build Folder | - | instant |

### Build Types

| Type | Command | Use When |
|------|---------|----------|
| **Debug** | `build.bat Debug` | Development, testing, debugging |
| **Release** | `build.bat Release` | Final builds, distribution, performance |

### What Rebuilds When?

| You Changed | Build | Why |
|-------------|-------|-----|
| `game/src/*.cpp` | Game | C++ needs recompiling |
| `assets/lua/*.lua` | Assets | Lua goes in asset pack |
| `assets/textures/` | Assets | Textures go in asset pack |
| `assets/models/` | Assets | Models go in asset pack |
| `src/Core/*.cpp` | All | Engine code changed |

---

## 🎓 Understanding the Build Output

### Console Messages Explained

**When you click "Build Game":**
```
[Core] Building Game.dll...
[Core] Executing: cd "C:/Portal" && build.bat Debug game
[1/4] Building Game.dll...
CMake configuring...
Compiler building...
SUCCESS: Game.dll compiled
[Core] Game.dll built successfully!
```

**What this means:**
- ✅ Your C++ code compiled without errors
- ✅ Game.dll is in game/build/Debug/
- ✅ Ready to restart engine and test

**When you click "Build Assets":**
```
[Core] Building GameAssets.apak...
[Core] Executing: cd "C:/Portal" && build.bat Debug assets
[3/4] Packing assets (GameAssets.apak)...
Processing: assets/textures/portal.png
Processing: assets/lua/door.lua
Processing: assets/scenes/chamber.s67
SUCCESS: GameAssets.apak created
  Size: 45,123,456 bytes
[Core] GameAssets.apak built successfully!
```

**What this means:**
- ✅ All assets packed into .apak file
- ✅ Includes textures, models, Lua, scenes
- ✅ Ready to reload scene and test

---

## ❌ Troubleshooting

### Build Fails

**"Build failed with exit code 1"**
```
Problem: Compilation errors in C++ code
Solution: Check console for error messages
          Fix syntax errors in game/src/
          Try building from command line for details
```

**"Game.dll not found"**
```
Problem: Build didn't complete
Solution: Check game/build/Debug/ exists
          Try: build.bat Debug game from command line
```

**"Asset packer not found"**
```
Problem: Tools not built
Solution: build.bat Debug tools
          Rebuilds the asset packer
```

### Menu Grayed Out

**"Building menu items are grayed out"**
```
Problem: No project loaded
Solution: File > Open Project first
          Must have a project open to build
```

### Build Takes Forever

**"Build stuck or taking too long"**
```
Problem: First build downloads dependencies
Solution: Wait 5-10 minutes on first build
          Subsequent builds are much faster (30 sec)
          
Problem: Something went wrong
Solution: Ctrl+C to cancel
          Check console for errors
```

---

## 💡 Pro Tips

### Tip 1: Use Keyboard Shortcuts

```
F7           → Build Game (fastest!)
Ctrl+F7      → Build All
Ctrl+S       → Save Level
F5           → Reload Scene (hot-reload Lua!)
```

### Tip 2: Check Console Output

The console at the bottom shows:
- Build progress
- Compiler output
- Success/failure messages
- File sizes

**Always check it after building!**

### Tip 3: Save Before Building

The editor auto-saves your scene before building, but it's good practice to:
```
1. Ctrl+S (Save Level)
2. F7 (Build Game)
3. Test
```

### Tip 4: Incremental Builds

**Don't rebuild everything unless you need to:**
- Changed only C++? → Build Game
- Changed only Lua/assets? → Build Assets
- Changed engine code? → Build All

**This saves time!**

### Tip 5: Hot Reload for Rapid Iteration

**For Lua development:**
```
1. Edit assets/lua/door.lua
2. Build Assets (10 sec)
3. F5 to reload scene
4. See changes INSTANTLY!
```

No engine restart needed for Lua changes!

---

## 🎯 Summary

### To build your game:

**In the Editor (Recommended):**
1. Open your project
2. Click **Building > Build Game** (F7)
3. Wait for build to complete
4. Restart engine or reload scene
5. Test your changes!

**From Command Line:**
```bash
build.bat Debug game      # Game code
build.bat Debug assets    # Assets & Lua
build.bat Debug all       # Everything
```

**Build produces:**
- `Game.dll` → Your compiled C++ game code
- `GameAssets.apak` → Your packed assets + Lua scripts

**Source67.exe loads both and runs your game!**

---

## 📚 Related Documentation

- **Understanding the system:** GAME_PROJECT_GUIDE.md
- **First-time setup:** QUICK_START_GUIDE.md
- **Visual workflows:** VISUAL_WORKFLOW.md
- **Getting started:** START_HERE_NEW_USER.md

---

**Happy Building!** 🎮✨

*Press F7 in the editor to build your game right now!*

# 🎮 Source67 Build & Run Workflow - Visual Guide

This visual guide shows you exactly how to build and run Source67, step by step.

---

## 📍 Where Am I? Where Do I Run Things?

```
┌─────────────────────────────────────────────────────────────────┐
│  YOUR COMPUTER                                                  │
│                                                                 │
│  C:\Users\YourName\Projects\                                    │
│  └── Source67\                    ← YOU ARE HERE!               │
│      ├── build.bat                ← Run this file               │
│      ├── RUN.bat                  ← Or run this after building  │
│      ├── CMakeLists.txt                                         │
│      ├── README.md                                              │
│      ├── QUICK_START_GUIDE.md                                   │
│      ├── src\                     ← Engine source code          │
│      ├── game\                    ← Your game code              │
│      ├── assets\                  ← Game assets                 │
│      └── ...                                                    │
│                                                                 │
│  ⚠️  IMPORTANT: Always run build.bat from this directory!       │
│      (The Source67 root folder, where build.bat is located)    │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 🔨 The Build Process (What Happens When You Run build.bat)

```
┌────────────────────────────────────────────────────────────────────────────┐
│                                                                            │
│  YOU:                                                                      │
│  ┌────────────────────┐                                                   │
│  │ Open Command Prompt│                                                   │
│  └─────────┬──────────┘                                                   │
│            │                                                               │
│            │ cd C:\Users\YourName\Projects\Source67                        │
│            ▼                                                               │
│  ┌──────────────────────┐                                                │
│  │ build.bat Debug all  │  ◄── Run this command                          │
│  └─────────┬────────────┘                                                │
│            │                                                               │
│            │  Build script starts...                                      │
│            ▼                                                               │
│                                                                            │
│  STEP 1: Build Game.dll                                                   │
│  ┌────────────────────────────────────────────────┐                      │
│  │ [1/4] Building Game.dll...                     │                      │
│  │ ----------------------------------------        │                      │
│  │ CMake configures...                            │                      │
│  │ Compiler builds C++ game code...               │                      │
│  │ ✓ SUCCESS: Game.dll compiled                   │                      │
│  │                                                 │                      │
│  │ Output: game\build\Debug\Game.dll              │                      │
│  └────────────────────────────────────────────────┘                      │
│            │                                                               │
│            │  (~1-2 minutes)                                              │
│            ▼                                                               │
│                                                                            │
│  STEP 2: Build Asset Packer                                               │
│  ┌────────────────────────────────────────────────┐                      │
│  │ [2/4] Building asset packer tool...            │                      │
│  │ ----------------------------------------        │                      │
│  │ CMake configures...                            │                      │
│  │ Compiler builds asset packer...                │                      │
│  │ ✓ SUCCESS: Asset packer built                  │                      │
│  │                                                 │                      │
│  │ Output: cmake-build-tools\Debug\               │                      │
│  │         asset_packer.exe                       │                      │
│  └────────────────────────────────────────────────┘                      │
│            │                                                               │
│            │  (~1 minute)                                                 │
│            ▼                                                               │
│                                                                            │
│  STEP 3: Pack Assets                                                      │
│  ┌────────────────────────────────────────────────┐                      │
│  │ [3/4] Packing assets (GameAssets.apak)...     │                      │
│  │ ----------------------------------------        │                      │
│  │ Reading assets from assets\ folder...          │                      │
│  │ - Scenes (.s67)                                │                      │
│  │ - Models (.obj)                                │                      │
│  │ - Textures (.png, .jpg)                        │                      │
│  │ - Shaders (.glsl)                              │                      │
│  │ - Lua scripts (.lua)                           │                      │
│  │ ✓ SUCCESS: GameAssets.apak created             │                      │
│  │   Size: 65,070,482 bytes                       │                      │
│  │                                                 │                      │
│  │ Output: GameAssets.apak (in root)              │                      │
│  └────────────────────────────────────────────────┘                      │
│            │                                                               │
│            │  (~10 seconds)                                               │
│            ▼                                                               │
│                                                                            │
│  STEP 4: Build Engine                                                     │
│  ┌────────────────────────────────────────────────┐                      │
│  │ [4/4] Building Source67 engine...              │                      │
│  │ ----------------------------------------        │                      │
│  │ CMake configures...                            │                      │
│  │ Fetching dependencies (first time only):       │                      │
│  │   - GLFW (window management)                   │                      │
│  │   - GLM (math library)                         │                      │
│  │   - spdlog (logging)                           │                      │
│  │   - Jolt Physics                               │                      │
│  │   - ImGui (UI)                                 │                      │
│  │   - sol2 (Lua bindings)                        │                      │
│  │ Compiler builds engine...                      │                      │
│  │ ✓ SUCCESS: Source67.exe built (Editor mode)    │                      │
│  │                                                 │                      │
│  │ Output: cmake-build-debug\Debug\               │                      │
│  │         Source67.exe                           │                      │
│  └────────────────────────────────────────────────┘                      │
│            │                                                               │
│            │  (~3-5 minutes first time, ~30 sec after)                   │
│            ▼                                                               │
│                                                                            │
│  ┌────────────────────────────────────────────────┐                      │
│  │ =========================================       │                      │
│  │ Build Complete!                                │                      │
│  │ =========================================       │                      │
│  │                                                 │                      │
│  │ To run: cmake-build-debug\Debug\Source67.exe   │                      │
│  │                                                 │                      │
│  │ Press any key to exit...                       │  ◄── Window stays    │
│  └────────────────────────────────────────────────┘      open now!       │
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘
```

**Total Time:** ~5-10 minutes first build, ~1-2 minutes subsequent builds

---

## 🎮 Running the Engine (After Building)

### Method 1: Use RUN.bat (Easiest)

```
┌──────────────────────────────────────────────────────┐
│  1. Double-click RUN.bat in the Source67 folder     │
│                                                      │
│  OR                                                  │
│                                                      │
│  2. In Command Prompt:                              │
│     C:\...\Source67> RUN.bat                        │
│                                                      │
│  → Automatically finds and runs the engine!         │
└──────────────────────────────────────────────────────┘
```

### Method 2: Run Directly

```
┌──────────────────────────────────────────────────────┐
│  In Command Prompt:                                 │
│  C:\...\Source67> cmake-build-debug\Debug\          │
│                    Source67.exe                      │
│                                                      │
│  OR                                                  │
│                                                      │
│  Navigate to cmake-build-debug\Debug\ and           │
│  double-click Source67.exe                          │
└──────────────────────────────────────────────────────┘
```

### What You'll See:

```
┌─────────────────────────────────────────────────────────────────┐
│                                                                 │
│  Console Window:                Terminal Output:               │
│  ┌─────────────────┐            ┌────────────────────────┐     │
│  │ [Core] Init...  │            │ [Core] Initializing... │     │
│  │ [Core] Loading..│            │ [Core] Loading DLL...  │     │
│  │ [Core] Ready!   │            │ [Core] Engine ready!   │     │
│  └─────────────────┘            └────────────────────────┘     │
│                                                                 │
│  Engine Window:                                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  File  Edit  View  Tools                    ▢  ▭  ✕     │  │
│  ├──────────────────────────────────────────────────────────┤  │
│  │  ▶  ❚❚  ■  │  [Transform: W] [Rotate: E] [Scale: R]    │  │
│  ├──────────┬──────────────────────┬───────────────────────┤  │
│  │          │  ┌────────────────┐  │                       │  │
│  │  Scene   │  │   3D View      │  │   Inspector           │  │
│  │  Hierarchy│  │   (Scene)      │  │   ┌─────────────┐   │  │
│  │  ┌──────┐│  │                │  │   │ Entity Name │   │  │
│  │  │Player││  │    ╱╲          │  │   ├─────────────┤   │  │
│  │  │Ground││  │   ╱  ╲         │  │   │ Transform   │   │  │
│  │  │Light ││  │  └────┘        │  │   │ Position    │   │  │
│  │  └──────┘│  │                │  │   │ X: 0.0      │   │  │
│  │          │  └────────────────┘  │   │ Y: 2.0      │   │  │
│  │          │                      │   │ Z: 0.0      │   │  │
│  │          │  ┌────────────────┐  │   └─────────────┘   │  │
│  │          │  │  Game View     │  │                       │  │
│  │          │  │  (Camera)      │  │                       │  │
│  ├──────────┴──┴────────────────┴──┴───────────────────────┤  │
│  │  Content Browser                                        │  │
│  │  📁 assets/  📁 scenes/  📁 models/  📄 test.s67       │  │
│  └──────────────────────────────────────────────────────────┘  │
│                                                                 │
│  ✓ If you see this window, everything is working!              │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## ❌ Common Mistakes & Solutions

### Mistake #1: Running from Wrong Directory

```
❌ WRONG:
C:\Users\YourName> build.bat
Error: build.bat not found!

C:\Users\YourName\Projects\Source67\game> build.bat
Error: Running from game\ subfolder - won't work!

✓ CORRECT:
C:\Users\YourName\Projects\Source67> build.bat Debug all
         └─── You should be HERE ───┘
```

### Mistake #2: Double-Clicking Without Reading Output

```
❌ BEFORE (old build.bat):
[Double-click build.bat]
→ Window appears
→ Build runs
→ Window closes immediately
→ Can't see if it worked or failed! 😵

✓ NOW (improved build.bat):
[Double-click build.bat]
→ Window appears
→ Build runs
→ Shows success or error messages
→ "Press any key to exit..." ← Window stays open! ✓
→ You can read what happened!
```

### Mistake #3: Not Having Prerequisites

```
❌ Common Error:
"cmake is not recognized as an internal or external command"

✓ Solution:
Install CMake from https://cmake.org/download/
Select "Add to PATH" during installation
Restart Command Prompt

❌ Common Error:
"No C++ compiler found"

✓ Solution:
Install Visual Studio 2022 (free Community edition)
Select "Desktop development with C++" workload
Restart and try again
```

---

## 🔄 Daily Workflow (Once Set Up)

### Scenario 1: Changed Game Code

```
1. Edit files in game\src\MyScript.cpp
2. Run: build.bat Debug game       ← Only rebuilds game DLL (~30 sec)
3. Run: RUN.bat                     ← Launch engine
4. Test your changes!
```

### Scenario 2: Added New Assets

```
1. Copy model.obj to assets\models\
2. Run: build.bat Debug assets      ← Only repacks assets (~10 sec)
3. Run: RUN.bat                     ← Launch engine
4. Load the new model in editor!
```

### Scenario 3: Changed Engine Code

```
1. Edit files in src\Core\Application.cpp
2. Run: build.bat Debug engine      ← Only rebuilds engine (~1 min)
3. Run: RUN.bat                     ← Launch engine
4. Test engine changes!
```

### Scenario 4: First Build / Clean Rebuild

```
1. Run: build.bat Debug all         ← Builds everything (~5-10 min)
2. Run: RUN.bat                     ← Launch engine
3. Start developing!
```

---

## 📚 Quick Command Reference

```
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│  Command                        What It Does                │
│  ────────────────────────────────────────────────────────   │
│  build.bat                      Build everything (Debug)    │
│  build.bat Debug all            Build everything (Debug)    │
│  build.bat Release all          Build everything (Release)  │
│  build.bat Debug game           Build only Game.dll         │
│  build.bat Debug assets         Pack only assets            │
│  build.bat Debug engine         Build only Source67.exe     │
│                                                             │
│  RUN.bat                        Run the built engine        │
│                                                             │
│  cmake-build-debug\Debug\       Run engine directly         │
│  Source67.exe                                               │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## 🎓 Understanding the Output

### Success Messages:

```
✓ SUCCESS: Game.dll compiled
✓ SUCCESS: Asset packer built
✓ SUCCESS: GameAssets.apak created
  Size: 65,070,482 bytes
✓ SUCCESS: Source67.exe built (Editor mode)

========================================
Build Complete!
========================================
```

### Error Messages:

```
ERROR: CMake configuration failed for Game.dll
→ Check that CMake is installed

ERROR: Game.dll compilation failed
→ Check for syntax errors in your C++ code

ERROR: Asset packing failed
→ Check that assets\ folder exists

BUILD FAILED!
→ Review the error messages above
→ See QUICK_START_GUIDE.md for help
```

---

## 🎯 Summary: The Answer to Your Question

**Q: "Where do I run build.bat from?"**

**A:** Always from the **Source67 root directory** (where build.bat is located).

```
✓ CORRECT:    C:\Users\YourName\Projects\Source67>
❌ WRONG:     C:\Users\YourName>
❌ WRONG:     C:\Users\YourName\Projects\Source67\game>
❌ WRONG:     C:\Users\YourName\Projects\Source67\src>
```

**Q: "Do I make my game with the engine, then place build.bat there?"**

**A:** No! build.bat is **already in the repository**. You don't move it or copy it. It's in the root folder where you cloned Source67. You write your game code in the `game/` folder, and build.bat compiles everything.

**Q: "How do I use it?"**

**A:** Three simple steps:
1. Open Command Prompt
2. Navigate to Source67 folder: `cd C:\Path\To\Source67`
3. Run: `build.bat Debug all`

Then run the engine with `RUN.bat` or `cmake-build-debug\Debug\Source67.exe`

---

**Need more help?** See `QUICK_START_GUIDE.md` for the complete tutorial!

# 🎮 Understanding Source67: Engine vs Your Game Project

**Confused about what goes where?** This guide explains the relationship between the Source67 engine and your actual game project.

---

## 🤔 The Core Concept

### Two Separate Things

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│  SOURCE67 ENGINE                    YOUR GAME PROJECT              │
│  (This Repository)                  (Separate Folder)              │
│                                                                     │
│  C:\Source67\                       C:\Desktop\Portal\             │
│  ├── src/                           ├── Portal.source              │
│  ├── game/                          ├── scripts/                   │
│  ├── assets/                        │   ├── player.cpp             │
│  ├── build.bat                      │   └── enemy.lua              │
│  └── Source67.exe (built)           ├── assets/                    │
│      ↓                               │   ├── textures/             │
│      │                               │   ├── models/               │
│      │                               │   └── scenes/               │
│      │                               └── build/                    │
│      │                                   ├── Game.dll              │
│      └───────────[loads]──────────────→  └── GameAssets.apak       │
│                                                                     │
│  The ENGINE                          Your GAME                     │
│  (One copy, reusable)                (Each project is unique)      │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

**Key Point:** Source67 is the **ENGINE** (like Unity, Unreal, or Godot). Your game "Portal" is a **PROJECT** that uses the engine.

---

## 📂 Current vs Future Setup

### Current Setup (Integrated Development)

**Right now, everything is in one place:**

```
Source67/                           ← Engine repository
├── src/                            ← Engine source code (C++)
├── game/                           ← Your game C++ code
│   └── src/
│       ├── game_api.cpp           ← Game logic in C++
│       └── Components/
├── assets/                         ← Your game assets
│   ├── textures/
│   ├── models/
│   └── lua/                        ← Game Lua scripts
├── build.bat                       ← Builds everything
└── cmake-build-debug/
    └── Debug/
        ├── Source67.exe            ← The engine
        ├── Game.dll                ← Your game (from game/)
        └── GameAssets.apak         ← Your assets (from assets/)
```

**This works for learning and simple projects.**

### Future Setup (Separate Game Project)

**For a real game, you'd want:**

```
C:\Users\YourName\
├── Source67/                       ← Engine (one copy)
│   ├── src/                        ← Engine code
│   ├── build.bat
│   └── bin/
│       └── Source67.exe            ← Built engine
│
└── Desktop/
    └── Portal/                     ← Your game project
        ├── Portal.source           ← Project manifest
        ├── src/                    ← C++ game code
        │   ├── player.cpp
        │   ├── portal_gun.cpp
        │   └── puzzle_logic.cpp
        ├── scripts/                ← Lua game scripts
        │   ├── door.lua
        │   └── button.lua
        ├── assets/                 ← Game assets
        │   ├── textures/
        │   │   ├── portal_blue.png
        │   │   └── portal_orange.png
        │   ├── models/
        │   │   ├── companion_cube.obj
        │   │   └── turret.obj
        │   ├── audio/
        │   └── scenes/
        │       └── test_chamber.s67
        ├── build.bat               ← Builds YOUR game
        └── bin/
            ├── Game.dll            ← Compiled from src/
            └── PortalAssets.apak   ← Packed from assets/
```

---

## 🏗️ What Goes Where? The Build Process Explained

### Part 1: C++ Game Code → Game.dll

**What:** Your C++ game logic  
**Where it lives:** `game/src/` or `Portal/src/`  
**What it becomes:** `Game.dll` (Dynamic Link Library)

```cpp
// Example: Portal/src/portal_gun.cpp
class PortalGun {
public:
    void ShootPortal(bool isBlue) {
        // Create portal at raycast hit point
        // Physics calculations
        // Mesh spawning
    }
    
    void Teleport(Player* player) {
        // Portal teleportation logic
    }
};

// Export to engine via game_api.cpp
void game_update(float deltaTime) {
    if (Input::MouseClicked(LEFT)) {
        portalGun->ShootPortal(true);
    }
}
```

**Build process:**
```cmd
# Your C++ code is compiled
Portal/src/*.cpp  →  [C++ Compiler]  →  Game.dll
```

**What's in the DLL:**
- ✅ Compiled C++ game logic
- ✅ Game systems (combat, inventory, AI)
- ✅ Custom components
- ✅ Physics behavior
- ❌ NOT assets (textures, models)
- ❌ NOT Lua scripts (those go in asset pack)

### Part 2: Assets + Lua → GameAssets.apak

**What:** Everything your game needs at runtime  
**Where it lives:** `assets/` folder  
**What it becomes:** `GameAssets.apak` (Binary archive)

```
Portal/assets/
├── textures/
│   └── portal_blue.png     ──┐
├── models/                   │
│   └── cube.obj              │
├── audio/                    ├──→  [Asset Packer]  →  PortalAssets.apak
│   └── portal_shoot.wav      │
├── scenes/                   │
│   └── chamber_01.s67        │
└── scripts/                  │
    └── button.lua           ──┘
```

**Build process:**
```cmd
asset_packer -i assets/ -o GameAssets.apak --include-lua
```

**What's in the .apak:**
- ✅ Textures (.png, .jpg)
- ✅ 3D Models (.obj)
- ✅ Audio files (.wav, .mp3)
- ✅ Shaders (.glsl)
- ✅ Scenes (.s67)
- ✅ Lua scripts (.lua)
- ✅ Fonts
- ❌ NOT C++ code (that's compiled to DLL)

### Part 3: Source67.exe (The Engine)

**What:** The runtime that runs your game  
**Where it lives:** Built from `Source67/src/`  
**What it does:** Loads Game.dll + GameAssets.apak and runs them

```
Source67.exe
    │
    ├─→ Loads Game.dll
    │   └─→ Calls game_initialize()
    │       Calls game_update() every frame
    │       Calls game_render() every frame
    │
    ├─→ Loads GameAssets.apak
    │   └─→ Extracts textures
    │       Loads models
    │       Executes Lua scripts
    │
    └─→ Provides services:
        ├─ Graphics rendering
        ├─ Physics simulation (Jolt)
        ├─ Input handling
        ├─ Audio playback
        └─ Scripting (Lua + C++)
```

---

## 🎯 The Complete Build & Run Flow

### Step 1: Build the Engine (Once)

```cmd
cd C:\Source67
build.bat Release all
```

Output:
- `cmake-build-release/Release/Source67.exe` ← The engine executable

**You only need to build the engine once** (or when updating it).

### Step 2: Build Your Game

**Option A: Current integrated setup**
```cmd
cd C:\Source67
build.bat Debug game     # Builds Game.dll from game/src/
build.bat Debug assets   # Packs assets/ into GameAssets.apak
```

**Option B: Future separate project**
```cmd
cd C:\Desktop\Portal
build.bat Debug          # Builds Game.dll and PortalAssets.apak
```

Output:
- `Game.dll` ← Your compiled C++ game code
- `GameAssets.apak` ← Your packed assets and Lua scripts

### Step 3: Run Your Game

```cmd
Source67.exe
```

**What happens at runtime:**

```
1. Source67.exe starts
   └─→ Initializes graphics, physics, input

2. Searches for Game.dll
   └─→ Finds it in ./game/build/Debug/ or ./
   └─→ Loads the DLL
   └─→ Calls game_initialize(engine_context, lua_state)

3. Searches for GameAssets.apak
   └─→ Finds it in ./
   └─→ Opens the binary file
   └─→ Reads the index table
   └─→ Extracts assets to memory

4. Main game loop:
   Every frame (60+ FPS):
   ├─→ Calls game_update(deltaTime)      [Your C++ code runs]
   ├─→ Updates Lua scripts                [Your Lua code runs]
   ├─→ Physics simulation
   ├─→ Calls game_render()                [Your C++ rendering]
   └─→ Draws to screen

5. When you close the window:
   └─→ Calls game_shutdown()
   └─→ Unloads Game.dll
   └─→ Closes GameAssets.apak
   └─→ Engine exits
```

---

## 💡 Understanding the Separation

### Why DLL for C++ Code?

**Game.dll contains:**
- Your game logic (C++)
- Can be reloaded without recompiling the engine
- Hot-reload support (change code, rebuild DLL, reload)
- Separate compilation = faster iteration

**Example:**
```cpp
// game/src/game_api.cpp
void game_update(float deltaTime) {
    // This code runs every frame
    // Changes here require rebuilding Game.dll
    // But NOT rebuilding Source67.exe!
    
    UpdatePortalGuns(deltaTime);
    CheckPuzzleSolved();
    UpdateCompanionCube();
}
```

### Why .apak for Assets?

**GameAssets.apak contains:**
- Binary packed format = fast loading
- All assets in one file = easy distribution
- Includes Lua scripts for hot-reload
- Compressed and indexed

**Benefits:**
- One file to distribute instead of thousands
- Faster loading than loose files
- Protection (not plain text)
- Easy to replace (swap .apak file = new assets)

---

## 🗂️ How to Structure Your Game Project

### Example: Making "Portal" Game

#### Step 1: Plan Your Project

```
Portal/                             What you create
├── Portal.source                   Project manifest (metadata)
├── README.md                       Your game's readme
├── src/                            C++ game code
│   ├── game_api.cpp               Required: engine callbacks
│   ├── portal_gun.h/cpp           Your game systems
│   ├── puzzle_manager.h/cpp
│   └── Components/
│       ├── portal.h/cpp
│       ├── button.h/cpp
│       └── cube.h/cpp
├── scripts/                        Lua game scripts
│   ├── door.lua
│   ├── button.lua
│   └── companion_cube.lua
├── assets/                         Game assets
│   ├── scenes/
│   │   ├── menu.s67
│   │   └── test_chamber_01.s67
│   ├── textures/
│   │   ├── portal_blue.png
│   │   └── portal_orange.png
│   ├── models/
│   │   ├── companion_cube.obj
│   │   ├── turret.obj
│   │   └── portal_gun.obj
│   ├── audio/
│   │   └── portal_open.wav
│   └── shaders/
│       └── portal_effect.glsl
└── build/                          Build output
    ├── Game.dll                    Built from src/
    └── PortalAssets.apak           Packed from assets/ + scripts/
```

#### Step 2: Write Game Code

**C++ (goes in src/ → becomes Game.dll):**

```cpp
// Portal/src/game_api.cpp
#include "portal_gun.h"
#include "puzzle_manager.h"

static PortalGun* g_PortalGun = nullptr;
static PuzzleManager* g_PuzzleManager = nullptr;

extern "C" {

void game_initialize(void* engine_context, void* lua_state) {
    g_PortalGun = new PortalGun();
    g_PuzzleManager = new PuzzleManager();
    
    // Load Lua scripts from asset pack
    // They'll be automatically executed
}

void game_update(float deltaTime) {
    // Your game loop
    g_PortalGun->Update(deltaTime);
    g_PuzzleManager->Update(deltaTime);
    
    // Input handling
    if (Input::MouseClicked(LEFT_BUTTON)) {
        g_PortalGun->ShootPortal(true);
    }
    if (Input::MouseClicked(RIGHT_BUTTON)) {
        g_PortalGun->ShootPortal(false);
    }
}

void game_render() {
    // Custom rendering if needed
    g_PortalGun->RenderPortals();
}

void game_shutdown() {
    delete g_PortalGun;
    delete g_PuzzleManager;
}

} // extern "C"
```

**Lua (goes in scripts/ → packed into .apak):**

```lua
-- Portal/scripts/button.lua
Button = {}

function Button:OnCreate()
    self.pressed = false
    self.connectedDoor = nil
end

function Button:OnUpdate(deltaTime)
    -- Check if player/cube is on button
    if self:CheckCollision() then
        if not self.pressed then
            self.pressed = true
            if self.connectedDoor then
                self.connectedDoor:Open()
            end
        end
    else
        self.pressed = false
    end
end
```

#### Step 3: Add Assets

```
Portal/assets/textures/portal_blue.png       ← Your texture files
Portal/assets/models/companion_cube.obj      ← Your 3D models
Portal/scripts/button.lua                    ← Your Lua scripts
```

#### Step 4: Build Your Game

**Create Portal/build.bat:**

```batch
@echo off
echo Building Portal...

REM Build C++ code to Game.dll
cd src
cmake -B build
cmake --build build --config Debug
move build\Debug\Game.dll ..\build\Game.dll

REM Pack assets and scripts
cd ..
asset_packer -i assets/ -i scripts/ -o build\PortalAssets.apak --include-lua

echo Portal built!
echo - build\Game.dll
echo - build\PortalAssets.apak
```

Run it:
```cmd
cd C:\Desktop\Portal
build.bat
```

#### Step 5: Run Your Game

Copy Source67.exe to your project folder, or add it to PATH, then:

```cmd
cd C:\Desktop\Portal\build
Source67.exe
```

Or place Game.dll and PortalAssets.apak next to Source67.exe:

```cmd
cd C:\Source67\cmake-build-debug\Debug
copy C:\Desktop\Portal\build\Game.dll .
copy C:\Desktop\Portal\build\PortalAssets.apak GameAssets.apak
Source67.exe
```

---

## 🎓 Frequently Asked Questions

### Q: Where does my C++ game code go?

**A:** In the `src/` folder of your project (or `game/src/` if using integrated setup).  
**Compiled to:** `Game.dll`  
**Used for:** Game logic, AI, systems, physics behavior, custom components

### Q: Where do my Lua scripts go?

**A:** In the `scripts/` or `assets/lua/` folder.  
**Packed into:** `GameAssets.apak`  
**Used for:** Entity behavior, UI logic, gameplay scripts, quick prototyping

### Q: Where do my textures/models go?

**A:** In the `assets/` folder (textures/, models/, audio/, etc.).  
**Packed into:** `GameAssets.apak`  
**Used for:** All visual and audio content

### Q: What is the .source file?

**A:** A project manifest file (JSON) containing metadata:
```json
{
  "name": "Portal",
  "version": "1.0.0",
  "company": "Aperture Science",
  "default_scene": "test_chamber_01.s67"
}
```

### Q: Do I need to rebuild the engine for each game?

**A:** No! Build Source67.exe once. Each game is just:
- Game.dll (your C++ code)
- GameAssets.apak (your assets)

### Q: Can I have multiple games?

**A:** Yes! One engine, many games:

```
C:\
├── Source67\
│   └── Source67.exe              ← One engine
├── Desktop\
│   ├── Portal\
│   │   ├── Game.dll              ← Portal game
│   │   └── PortalAssets.apak
│   ├── HalfLife3\
│   │   ├── Game.dll              ← Half-Life 3 game
│   │   └── HL3Assets.apak
│   └── Minecraft2\
│       ├── Game.dll              ← Minecraft 2 game
│       └── MC2Assets.apak
```

Each uses the same Source67.exe but with different DLLs and asset packs!

### Q: What if I only use Lua (no C++)?

**A:** You still need a minimal Game.dll that just loads Lua scripts. You can have an empty `game_update()` and do everything in Lua.

### Q: What about the manifest.source?

**A:** It's optional metadata. The engine looks for it to know project info (name, default scene, etc.). If not present, it still works.

---

## 🎯 Summary: What Goes Where

| Content Type | Location | Becomes | Purpose |
|--------------|----------|---------|---------|
| C++ game code | `src/` or `game/src/` | `Game.dll` | Core game logic, systems, performance-critical code |
| Lua scripts | `scripts/` or `assets/lua/` | Inside `GameAssets.apak` | Entity behavior, UI, rapid prototyping |
| Textures | `assets/textures/` | Inside `GameAssets.apak` | Visual content |
| 3D Models | `assets/models/` | Inside `GameAssets.apak` | Meshes, geometry |
| Scenes | `assets/scenes/` | Inside `GameAssets.apak` | Level data (.s67 files) |
| Audio | `assets/audio/` | Inside `GameAssets.apak` | Sound effects, music |
| Shaders | `assets/shaders/` | Inside `GameAssets.apak` | Custom rendering |
| Engine code | `Source67/src/` | `Source67.exe` | The engine itself |

---

## 🚀 Next Steps

1. **For now:** Use the integrated setup (Source67/game/ and Source67/assets/)
2. **Learn:** Understand how Game.dll and GameAssets.apak work
3. **Experiment:** Modify game/src/game_api.cpp and rebuild
4. **Later:** Create a separate game project folder when you're ready

---

**Still confused?** Think of it like this:

- **Source67.exe** = Unity Editor or Unreal Engine
- **Game.dll** = Your compiled game code (C# scripts in Unity)
- **GameAssets.apak** = Your Assets folder (textures, models, etc.)
- **Your project folder** = A Unity/Unreal project folder

The engine loads your DLL and asset pack, just like Unity loads your project!

---

**For more details:**
- See `BUILD_SYSTEM_README.md` for build system internals
- See `QUICK_START_GUIDE.md` for step-by-step building
- See `game/src/game_api.h` for the complete API

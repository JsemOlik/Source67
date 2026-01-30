# 📖 Documentation Map - Where to Start

**New to Source67?** Here's where to find what you need:

---

## 🎯 I Want To...

### → **Publish my game on Steam, itch.io, etc.**
See: **[DISTRIBUTION.md](DISTRIBUTION.md)** 📦 **NEW!**

Complete guide to packaging and distributing your game:
- How to create a distributable game package
- What files are needed (just 3 essential files!)
- Steam publishing workflow
- itch.io, GOG, Epic Games Store
- Automated packaging scripts
- Platform-specific builds (Windows, Linux, macOS)

**Time:** 15 minutes to read, ready to publish!

---

### → **Build my game (easy in-editor building!)**
See: **[BUILDING.md](BUILDING.md)** 🔨

Essential guide for building your game:
- Click "Building > Build Game" in the editor (F7)
- Visual diagrams of what gets built
- Step-by-step tutorials
- Daily workflow examples
- Troubleshooting build issues

**Time:** 10 minutes to read, instant to use!

---

### → **Understand what this engine is and how game projects work**
See: **[GAME_PROJECT_GUIDE.md](GAME_PROJECT_GUIDE.md)** 🎮

Essential reading that explains:
- The difference between the engine and your game project
- **Lua vs C++ scripting** - when to use each
- What goes in Game.dll vs GameAssets.apak
- How to structure your own game folder
- The complete build and run process

**Time:** 20 minutes to read (worth it!)

---

### → **Build and run the engine for the first time**
See: **[QUICK_START_GUIDE.md](QUICK_START_GUIDE.md)** ⭐

This comprehensive guide covers:
- Prerequisites and installation
- First build (step-by-step)
- Running the engine
- Troubleshooting common issues

**Time:** 15 minutes to read, 5-10 minutes for first build

---

### → **See visual diagrams of the build process**
See: **[VISUAL_WORKFLOW.md](VISUAL_WORKFLOW.md)** 📊

Includes:
- ASCII diagrams showing where to run commands
- Visual representation of the 4-step build process
- Common mistakes illustrated
- Daily workflow scenarios

**Time:** 5 minutes to read

---

### → **Understand the build system in detail**
See: **[BUILD_SYSTEM_README.md](BUILD_SYSTEM_README.md)** 🔧

Covers:
- Hybrid build system architecture
- Game DLL development
- Asset packing
- Lua scripting integration
- Advanced build options

**Time:** 20 minutes to read

---

### → **Learn about the engine architecture**
See: **[README.md](README.md)** 📚

Includes:
- Engine features overview
- C++ scripting system
- Entity component system
- Console commands
- Player controller

**Time:** 10 minutes to read

---

### → **Get the integration working**
See: **[INTEGRATION_SUCCESS.md](INTEGRATION_SUCCESS.md)** ✅

Details:
- How the hybrid build system was integrated
- What changes were made
- Runtime behavior
- Testing steps

**Time:** 10 minutes to read

---

## 🚀 Quick Reference

### Build the Engine (Windows)
```cmd
# Open Command Prompt in Source67 folder
build.bat Debug all
```

### Run the Engine
```cmd
# After building
RUN.bat

# Or directly
cmake-build-debug\Debug\Source67.exe
```

### Common Commands
```cmd
build.bat Debug all      # Build everything
build.bat Debug game     # Just rebuild game code
build.bat Debug assets   # Just repack assets
```

---

## 🗂️ File Structure Reference

```
Source67/                           Your project root
├── build.bat                       ← Run this to build
├── RUN.bat                         ← Run this after building
├── QUICK_START_GUIDE.md            ← Start here if new
├── VISUAL_WORKFLOW.md              ← Visual diagrams
├── BUILD_SYSTEM_README.md          ← Build system details
├── README.md                       ← Engine overview
├── INTEGRATION_SUCCESS.md          ← Integration docs
│
├── src/                            Engine source code
│   ├── Core/                       Application, Window, Input
│   ├── Renderer/                   Scene, Camera, Mesh
│   ├── Physics/                    Jolt integration
│   └── ...
│
├── game/                           Your game code here
│   ├── src/
│   │   ├── game_api.cpp           Game DLL exports
│   │   └── Components/             Game components
│   └── build/                      Built Game.dll here
│
├── assets/                         Game assets
│   ├── scenes/                     .s67 scene files
│   ├── models/                     .obj models
│   ├── textures/                   .png, .jpg images
│   ├── shaders/                    .glsl shaders
│   └── lua/                        .lua scripts
│
├── cmake-build-debug/              Debug build output
│   └── Debug/
│       └── Source67.exe            ← Engine executable here
│
└── GameAssets.apak                 Packed assets file
```

---

## ❓ FAQ

**Q: Where do I put my game (like "Portal")?**  
A: **Option 1 (Current/Easy):** Use the integrated setup - put code in `game/src/` and assets in `assets/`. **Option 2 (Advanced):** Create a separate folder (e.g., Desktop/Portal/) with its own src/, assets/, build.bat. See **GAME_PROJECT_GUIDE.md** for full details.

**Q: What goes in the .dll file?**  
A: Your C++ game code from `game/src/` or your project's `src/` folder. This is compiled game logic, not assets. See **GAME_PROJECT_GUIDE.md** for details.

**Q: What goes in the .apak file?**  
A: All your assets (textures, models, audio) AND Lua scripts from the `assets/` folder. See **GAME_PROJECT_GUIDE.md** for complete breakdown.

**Q: Do I need to rebuild the engine for each game?**  
A: No! Build Source67.exe once. Each game is just its own Game.dll + GameAssets.apak. See **GAME_PROJECT_GUIDE.md**.

**Q: Can I have multiple games?**  
A: Yes! One engine (Source67.exe), many games (each with its own Game.dll and assets). See **GAME_PROJECT_GUIDE.md**.

**Q: Where do I run build.bat from?**  
A: Always from the `Source67` root directory (where build.bat is located).

**Q: The window closes immediately when I double-click build.bat**  
A: The improved build.bat now pauses at the end! But it's still better to run from Command Prompt to see all output.

**Q: How do I know if the build worked?**  
A: You'll see "Build Complete!" and file sizes at the end. Or check if `cmake-build-debug\Debug\Source67.exe` exists.

**Q: Where is my game code?**  
A: Put your game C++ code in the `game/src/` folder. It compiles to `Game.dll`.

**Q: Where do I put assets?**  
A: Place assets in the `assets/` folder. Run `build.bat Debug assets` to pack them.

**Q: How do I run the engine after building?**  
A: Run `RUN.bat` or run `cmake-build-debug\Debug\Source67.exe` directly.

**Q: Do I need to rebuild everything when I change game code?**  
A: No! Just run `build.bat Debug game` to rebuild only the game DLL (~30 seconds).

**Q: How do I publish my game on Steam/itch.io?**  
A: Build with Release mode, package the 3 files (YourGame.exe, Game.dll, GameAssets.apak), and distribute! See **DISTRIBUTION.md** for complete guide.

**Q: What files do I need to distribute my game?**  
A: Just 3 files: Your renamed Source67.exe (e.g., MyGame.exe), Game.dll, and GameAssets.apak. Add README.txt and LICENSE.txt. See **DISTRIBUTION.md**.

---

## 🎓 Learning Path

### Day 1: Getting Started
1. Read **GAME_PROJECT_GUIDE.md** to understand the architecture (20 min)
2. Read QUICK_START_GUIDE.md (15 min)
3. Build the engine (5-10 min)
4. Run and explore the editor (15 min)

### Day 2: Understanding the System
1. Read README.md engine overview (10 min)
2. Explore the example scene
3. Try the console commands (press `~`)

### Day 3: Making Your First Changes
1. Edit game/src/game_api.cpp
2. Rebuild: `build.bat Debug game`
3. Run and test your changes

### Week 1+: Deep Dive
1. Read BUILD_SYSTEM_README.md
2. Write custom game components
3. Create Lua scripts
4. Build your game!

---

## 🆘 Getting Help

### If Something's Not Working

1. **Check the console output** - error messages tell you what's wrong
2. **Read the Troubleshooting section** in QUICK_START_GUIDE.md
3. **Check these common issues:**
   - Running from wrong directory → Must be Source67 root
   - CMake not installed → Install from cmake.org
   - No compiler → Install Visual Studio 2022
4. **Still stuck?** File a GitHub issue with:
   - Your OS and compiler
   - The exact command you ran
   - The complete error message

---

## 📝 Summary

**To build Source67:**
1. Open Command Prompt
2. Navigate to Source67 folder
3. Run `build.bat Debug all`
4. Wait 5-10 minutes (first time)
5. Run `RUN.bat` or `cmake-build-debug\Debug\Source67.exe`

**Files you need to know about:**
- `build.bat` - Builds everything
- `RUN.bat` - Runs the engine
- `game/src/` - Your game code
- `assets/` - Your game assets

**Documents to read:**
- QUICK_START_GUIDE.md (start here)
- VISUAL_WORKFLOW.md (visual learners)
- BUILD_SYSTEM_README.md (details)
- README.md (engine overview)

---

**Ready to start?** → [QUICK_START_GUIDE.md](QUICK_START_GUIDE.md) ⭐

**Happy Game Development!** 🎮✨

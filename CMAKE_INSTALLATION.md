# CMake Installation Guide for Source67

**CMake is required for building Game.dll from your C++ game code.**

---

## 🎯 Why Do I Need CMake?

When you click **Building > Build Game** in the Source67 editor, the engine needs to compile your C++ game code into a DLL file. CMake is the industry-standard build tool that makes this possible.

**Without CMake:**
- ❌ Cannot build Game.dll
- ❌ Cannot use C++ for game logic
- ✅ Can still use Lua scripts
- ✅ Can still pack assets (GameAssets.apak)

**With CMake:**
- ✅ Full C++ game development
- ✅ Build Game.dll from editor
- ✅ Complete game building workflow

---

## 📦 Installation Options

### Option 1: Installer Helper Script (Easiest)

**If you installed Source67 with the installer:**

1. Navigate to: `C:\Program Files\Source67\Tools\`
2. Double-click: `install_cmake.bat`
3. Follow the instructions

The script will:
- Check if CMake is already installed
- Open the CMake download page if needed
- Guide you through installation

---

### Option 2: Manual Download (Recommended)

**Windows:**

1. Visit: https://cmake.org/download/
2. Download: **cmake-3.28.1-windows-x86_64.msi** (or latest version)
3. Run the installer
4. **IMPORTANT:** Check "Add CMake to system PATH for all users"
5. Click Install

**Verify installation:**
```cmd
cmake --version
```

Should show: `cmake version 3.28.1` (or your version)

---

### Option 3: Package Managers

**Chocolatey (Windows):**
```cmd
choco install cmake
```

**Winget (Windows 10/11):**
```cmd
winget install Kitware.CMake
```

**Homebrew (macOS):**
```bash
brew install cmake
```

**APT (Ubuntu/Debian):**
```bash
sudo apt update
sudo apt install cmake
```

---

## ✅ Verification

**After installing CMake:**

1. **Close and reopen** any command prompts
2. **Restart Source67 editor** if it was running
3. Open a new command prompt
4. Run:
   ```cmd
   cmake --version
   ```

**Expected output:**
```
cmake version 3.28.1
CMake suite maintained and supported by Kitware (kitware.com/cmake)
```

---

## 🎮 Using CMake with Source67

### Building from Editor

**Once CMake is installed:**

1. Open Source67 editor
2. Open or create a project
3. Add C++ code to `game/src/`
4. Click **Building > Build Game** (F7)

**Console output:**
```
[BuildSystem] Building Game.dll...
[BuildSystem] Configuring CMake for Game.dll...
[BuildSystem] Compiling Game.dll...
[BuildSystem] Game.dll built successfully!
[BuildSystem] Output: C:\Users\YourName\MyProject\build\Debug\Game.dll
```

---

## 🔧 Troubleshooting

### "CMake is not installed or not in PATH"

**Problem:** CMake is installed but Source67 can't find it.

**Solutions:**

1. **Add CMake to PATH manually:**
   - Right-click "This PC" → Properties
   - Advanced system settings → Environment Variables
   - Edit "Path" under System variables
   - Add: `C:\Program Files\CMake\bin`
   - Click OK, restart Source67

2. **Reinstall CMake:**
   - Uninstall current CMake
   - Download from https://cmake.org/download/
   - During installation, check "Add to PATH"

3. **Verify PATH:**
   ```cmd
   echo %PATH%
   ```
   Should include `C:\Program Files\CMake\bin`

### "CMake version too old"

Source67 requires CMake 3.20 or newer.

**Check version:**
```cmd
cmake --version
```

**If too old:**
- Download latest from https://cmake.org/download/
- Install (it will upgrade your existing version)

### Build still fails after installing CMake

1. **Restart Source67 editor** (important!)
2. Check console output for specific error
3. Ensure `game/` folder has `CMakeLists.txt`
4. Check that game code compiles without errors

---

## 📋 Quick Reference

| Action | Command |
|--------|---------|
| Check if installed | `cmake --version` |
| Check PATH | `echo %PATH%` (Windows) or `echo $PATH` (Linux/Mac) |
| Download | https://cmake.org/download/ |
| Helper script | `C:\Program Files\Source67\Tools\install_cmake.bat` |

---

## 🎓 More Information

**What is CMake?**
- Industry-standard build system generator
- Used by: Unreal Engine, Godot, many AAA games
- Cross-platform (Windows, Linux, macOS)

**CMake vs Build.bat:**
- Old: build.bat called CMake externally
- New: Source67 calls CMake directly from C++
- Both require CMake to be installed

**Do I need Visual Studio?**
- No! CMake can use various compilers
- Windows: CMake can use MSVC (Visual Studio) or MinGW
- Linux: CMake uses GCC
- macOS: CMake uses Clang

---

## ✅ You're Ready!

Once CMake is installed and verified:
1. ✅ Open Source67 editor
2. ✅ Create or open a game project
3. ✅ Click Building > Build Game
4. ✅ Start making games!

**Happy Game Development!** 🎮

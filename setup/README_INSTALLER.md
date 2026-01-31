# Source67 Installer Build Guide

This folder contains the NSIS installer script and build tools for creating the Source67 Windows installer.

## 📋 Prerequisites

Before building the installer, you need:

### 1. Built Source67 Engine (RELEASE BUILD!)

**⚠️ IMPORTANT: Always use Release builds for distribution, NOT Debug builds!**

**The engine MUST be built in Release mode first!**

```cmd
# From Source67 root directory
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --config Release
```

This creates:
- `cmake-build-release/Source67.exe` (Windows)
- `assets/` folder with all engine assets (copied to build folder)

**Why Release and not Debug?**
- Release builds are optimized and fast
- Release builds are much smaller (no debug symbols)
- Release builds don't require debug C runtime DLLs
- Debug builds may crash on systems without Visual Studio
- Debug builds should NEVER be distributed to end users

**Note:** Source67 uses static linking, so no DLL files are needed!

### 2. NSIS (Nullsoft Scriptable Install System)

**Download:** https://nsis.sourceforge.io/Download

**Install:** Use the default installer, it will add `makensis` to your PATH.

**Verify installation:**
```cmd
makensis /VERSION
```

Should show: `v3.x` (any version 3.x is fine)

---

## 🔨 Building the Installer

### Quick Method

**Double-click:** `build_installer.bat`

The script will:
1. ✅ Check if NSIS is installed
2. ✅ Verify Source67.exe exists in build directory
3. ✅ Build the installer with `makensis`
4. ✅ Show success/error message

**Output:** `Source67_Setup.exe` (in this folder)

### Manual Method

```cmd
cd setup
makensis Source67.nsi
```

---

## 📂 What Gets Packaged

The installer includes:

### Required Files (Always)
- ✅ `Source67.exe` - Main engine executable (Release build)
- ✅ `assets/**` - All engine assets (shaders, fonts, icons, etc.)

**Note:** Source67 uses static linking, so no external DLL files are needed!
All dependencies (GLFW, ImGui, Jolt Physics, etc.) are compiled directly into the executable.

### Optional Components (User Choice)
- ✅ **Desktop Shortcut** - Shortcut on user's desktop
- ✅ **CMake Build Tools** - Helper scripts for CMake installation
- ✅ **File Associations** - `.source` and `.s67` file associations

### Installation Location
Default: `C:\Program Files\Source67\`

---

## 🎯 Build Directory Detection

The installer script expects a **Release build** at:
- `cmake-build-release/Source67.exe`

If the file doesn't exist, `build_installer.bat` will show an error and exit.

**Always build Release version before creating installer:**
```cmd
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --config Release
```

---

## 📝 Installer Script Details

### Source67.nsi

**Main sections:**

```nsis
Section "Main Engine (Required)"
  - Copies Source67.exe (Release build)
  - Copies assets folder
  - Creates registry keys
  - Creates shortcuts
SectionEnd

Section "CMake Build Tools (Recommended)"
  - Creates install_cmake.bat helper
  - Creates CMAKE_INFO.txt guide
SectionEnd

Section "File Associations (.source, .s67)"
  - Associates project files with Source67
  - Allows double-clicking to open
SectionEnd
```

### Key Features

- ✅ **Release build packaging** - Uses optimized Release builds
- ✅ **Static linking** - No DLL dependencies needed
- ✅ **Validation** - Checks if files exist before packaging
- ✅ **Smart defaults** - Installs to Program Files
- ✅ **Uninstaller** - Clean removal of all files

---

## 🔧 Troubleshooting

### "Source67.exe not found"

**Problem:** Engine hasn't been built in Release mode yet.

**Solution:**
```cmd
# From Source67 root
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --config Release
```

### "makensis is not recognized"

**Problem:** NSIS not installed or not in PATH.

**Solution:**
1. Download NSIS from https://nsis.sourceforge.io/
2. Install with default settings
3. Close and reopen command prompt
4. Try again

### Installer builds but exe doesn't run

**Problem:** Using Debug build instead of Release build.

**Solution:**
Debug builds require debug C runtime DLLs that aren't on most systems. Always use Release builds:
```cmd
# Build Release version
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --config Release

# Rebuild installer
cd setup
build_installer.bat
```

### "abort() has been called" when running installed exe

**Root Cause:**
The installer was packaging a **Debug build** instead of a **Release build**. Debug builds:
1. Are much larger (include debug symbols)
2. May depend on debug C runtime DLLs not present on user systems
3. Are not optimized and run slowly
4. Should never be distributed to end users

**Solution (FIXED):**
1. The NSIS script now uses `cmake-build-release\Source67.exe` (Release build)
2. `build_installer.bat` now checks for Release build instead of Debug
3. Source67 uses static linking, so no DLLs are needed

**To build the installer correctly:**
```cmd
# First, build Release version (NOT Debug!)
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --config Release

# Then build the installer
cd setup
build_installer.bat
```

**Debug steps if still having issues:**
1. Verify Release build exists: `dir ..\cmake-build-release\Source67.exe`
2. Test the Release exe directly before packaging:
   ```cmd
   cd cmake-build-release
   Source67.exe
   ```
3. If that works, rebuild the installer
4. If it doesn't work, check that assets folder exists in cmake-build-release

---

## 📦 Distribution

### For End Users

**Just send:** `Source67_Setup.exe`

Users run it and get:
- ✅ Source67 installed in Program Files
- ✅ Start Menu shortcuts
- ✅ Optional desktop shortcut
- ✅ CMake installation helper
- ✅ File associations for .source and .s67 files

### For Developers

If distributing source code, include:
- Full repository
- Build instructions in main README
- This installer build guide

---

## 🎮 After Installation

Users can:

1. **Run from Start Menu**
   - Start → Source67 → Source67 Engine

2. **Run from Desktop** (if they chose that option)
   - Double-click Source67 Engine shortcut

3. **Install CMake** (for game building)
   - Navigate to `C:\Program Files\Source67\Tools\`
   - Run `install_cmake.bat`
   - Follow instructions

4. **Double-click .source files** (if they chose file associations)
   - Opens Source67 with that project loaded

---

## 🔄 Updating the Installer

When you update the engine:

1. **Rebuild the engine (Release mode)**
   ```cmd
   cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
   cmake --build cmake-build-release --config Release
   ```

2. **Rebuild the installer**
   ```cmd
   cd setup
   build_installer.bat
   ```

3. **Test the new installer**
   - Uninstall old version
   - Install new version
   - Verify it works

4. **Distribute**
   - Upload to website
   - Share with users

---

## ✅ Checklist

Before building the installer:

- [ ] Engine is built in **Release mode** (`cmake-build-release/Source67.exe` exists)
- [ ] NSIS is installed (`makensis /VERSION` works)
- [ ] Assets folder is up to date
- [ ] All changes are committed to git
- [ ] Version number updated if needed

After building the installer:

- [ ] `Source67_Setup.exe` created successfully
- [ ] Test installer on clean system (or VM)
- [ ] Verify installed engine runs without errors
- [ ] Check all shortcuts work
- [ ] Test file associations
- [ ] Verify CMake helper scripts exist

---

## 📚 More Information

**NSIS Documentation:** https://nsis.sourceforge.io/Docs/

**Main README:** ../README.md

**Build System Guide:** ../BUILDING.md

**Distribution Guide:** ../DISTRIBUTION.md

---

## 🆘 Getting Help

If you have issues building the installer:

1. Check this README
2. Ensure all prerequisites are met
3. Try manual build to see detailed errors
4. Check NSIS documentation
5. Ask in Source67 community/issues

**Happy Distributing!** 🚀

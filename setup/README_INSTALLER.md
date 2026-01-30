# Source67 Installer Build Guide

This folder contains the NSIS installer script and build tools for creating the Source67 Windows installer.

## 📋 Prerequisites

Before building the installer, you need:

### 1. Built Source67 Engine

**The engine MUST be built first!**

```cmd
# From Source67 root directory
cmake -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug --config Debug
```

This creates:
- `cmake-build-debug/Debug/Source67.exe` (Windows)
- `cmake-build-debug/Debug/*.dll` (if any shared libraries)
- `assets/` folder with all engine assets

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
- ✅ `Source67.exe` - Main engine executable
- ✅ `assets/**` - All engine assets (shaders, fonts, icons, etc.)
- ✅ `*.dll` - Any DLL dependencies (spdlog, etc.)

### Optional Components (User Choice)
- ✅ **Desktop Shortcut** - Shortcut on user's desktop
- ✅ **CMake Build Tools** - Helper scripts for CMake installation
- ✅ **File Associations** - `.source` and `.s67` file associations

### Installation Location
Default: `C:\Program Files\Source67\`

---

## 🎯 Build Directory Detection

The installer script intelligently searches for Source67.exe:

1. **Try:** `cmake-build-debug/Debug/Source67.exe` (MSVC Debug)
2. **Try:** `cmake-build-debug/Release/Source67.exe` (MSVC Release)
3. **Try:** `cmake-build-debug/Source67.exe` (MinGW/Makefile)
4. **Error:** If none found, show error and abort

**Also copies:**
- All `*.dll` files from the same directory
- Handles both Debug and Release builds

---

## 📝 Installer Script Details

### Source67.nsi

**Main sections:**

```nsis
Section "Main Engine (Required)"
  - Copies Source67.exe
  - Copies all DLL files
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

- ✅ **Multi-build support** - Detects Debug/Release builds
- ✅ **DLL bundling** - Automatically includes dependencies
- ✅ **Validation** - Checks if files exist before packaging
- ✅ **Smart defaults** - Installs to Program Files
- ✅ **Uninstaller** - Clean removal of all files

---

## 🔧 Troubleshooting

### "Source67.exe not found"

**Problem:** Engine hasn't been built yet.

**Solution:**
```cmd
# From Source67 root
cmake -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug --config Debug
```

### "makensis is not recognized"

**Problem:** NSIS not installed or not in PATH.

**Solution:**
1. Download NSIS from https://nsis.sourceforge.io/
2. Install with default settings
3. Close and reopen command prompt
4. Try again

### Installer builds but exe doesn't run

**Problem:** Missing DLL dependencies.

**Solution:**
The updated installer (this version) automatically copies DLLs. Make sure you:
1. Rebuilt the installer after building the engine
2. Used the updated `Source67.nsi` script
3. Check that DLLs exist in `cmake-build-debug/Debug/`

### "abort() has been called" when running installed exe

**Possible causes:**
1. ❌ Missing DLLs - Fixed by this update
2. ❌ Assets not found - Check `C:\Program Files\Source67\assets\`
3. ❌ Wrong build configuration - Try Release build instead of Debug

**Debug steps:**
1. Check if `assets/` folder exists in install directory
2. Check if any `.dll` files are in install directory
3. Try running from build directory first:
   ```cmd
   cmake-build-debug\Debug\Source67.exe
   ```
4. If that works but installer doesn't, rebuild installer

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

1. **Rebuild the engine**
   ```cmd
   cmake --build cmake-build-debug --config Debug
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

- [ ] Engine is built (`cmake-build-debug/Debug/Source67.exe` exists)
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

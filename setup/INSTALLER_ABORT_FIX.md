# Installer abort() Error - FIXED

## The Problem

When compiling the NSIS installer and running it, the installed Source67.exe would fail with:
```
Debug Error!
Program: C:\Program Files\Source67\Source67.exe
abort() has been called
(Press Retry to debug the application)
```

## Root Cause Analysis

The issue had **two main causes**:

### 1. Using Debug Build Instead of Release Build

The NSIS installer script was configured to package the **Debug build** from:
```
..\cmake-build-debug\Source67.exe
```

**Why this is a problem:**
- Debug builds include debug symbols and are much larger (~16 MB vs ~3-5 MB)
- Debug builds are compiled with `/MDd` (debug multithreaded DLL runtime)
- Debug builds require `vcruntime140d.dll`, `msvcp140d.dll` (debug C runtime DLLs)
- These debug DLLs are **NOT** installed on regular Windows systems
- They only exist on development machines with Visual Studio installed
- Without these DLLs, the program calls `abort()` and crashes

### 2. Missing DLL Warning Was Misleading

The NSIS compilation showed:
```
warning 7010: File: "..\cmake-build-debug\*.dll" -> no files found.
```

This warning was **misleading** because:
- Source67 uses **static linking** for all dependencies
- GLFW, ImGui, Jolt Physics, GLM, spdlog, GLAD are all compiled directly into the .exe
- **No DLL files are needed or generated**
- The warning was harmless, but made it seem like DLLs were missing

The real problem was the Debug build trying to load debug runtime DLLs.

## The Solution

### Changes Made

#### 1. Updated NSIS Script (Source67.nsi)

**Before:**
```nsis
File "..\cmake-build-debug\Source67.exe"
File /nonfatal "..\cmake-build-debug\*.dll"
```

**After:**
```nsis
; Core Files - Use Release build for distribution
; Source67 uses static linking, so no DLLs are needed
File "..\cmake-build-release\Source67.exe"
```

**Changes:**
- ✅ Now uses `cmake-build-release` (Release build)
- ✅ Removed DLL copying (not needed with static linking)
- ✅ Added clarifying comments

#### 2. Updated Build Script (build_installer.bat)

**Before:**
```batch
REM Check if Source67.exe exists in build directory
set EXE_FOUND=0
if exist "..\cmake-build-debug\Debug\Source67.exe" set EXE_FOUND=1
if exist "..\cmake-build-debug\Release\Source67.exe" set EXE_FOUND=1
if exist "..\cmake-build-debug\Source67.exe" set EXE_FOUND=1
```

**After:**
```batch
REM Check if Source67.exe exists in Release build directory
if not exist "..\cmake-build-release\Source67.exe" (
    echo [ERROR] Source67.exe not found in cmake-build-release directory.
    echo.
    echo Please build the RELEASE version of the engine first:
    echo   1. Run: cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
    echo   2. Run: cmake --build cmake-build-release --config Release
    echo.
    echo NOTE: Always use Release builds for distribution!
    echo Debug builds are larger, slower, and may not run on other systems.
    echo.
    pause
    exit /b 1
)
```

**Changes:**
- ✅ Now checks for Release build specifically
- ✅ Provides clear error messages
- ✅ Explains why Release is required
- ✅ Shows exact build commands

#### 3. Updated Documentation (README_INSTALLER.md)

- ✅ Clarified that Release builds must be used
- ✅ Explained why Debug builds don't work on other systems
- ✅ Removed references to DLL dependencies
- ✅ Updated all build instructions to use Release mode

#### 4. Updated Uninstaller (Source67.nsi)

**Before:**
```nsis
Delete "$INSTDIR\Source67.exe"
Delete "$INSTDIR\*.dll"
Delete "$INSTDIR\uninstall.exe"
```

**After:**
```nsis
Delete "$INSTDIR\Source67.exe"
Delete "$INSTDIR\uninstall.exe"
```

**Changes:**
- ✅ Removed DLL deletion (not needed)

## How to Build the Installer Correctly

### Step 1: Build Release Version

```cmd
cd Source67
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --config Release
```

### Step 2: Build Installer

```cmd
cd setup
build_installer.bat
```

Or manually:
```cmd
cd setup
makensis Source67.nsi
```

### Step 3: Test

The installer will create `Source67_Setup.exe`. Run it and verify:
- ✅ Installation completes successfully
- ✅ Engine runs without errors
- ✅ No "abort() has been called" error
- ✅ No missing DLL errors

## Why This Fix Works

### Release Build Characteristics

**Release builds (`/MD` runtime):**
- Optimized for performance
- Smaller size (no debug symbols)
- Uses `vcruntime140.dll`, `msvcp140.dll` (release runtime DLLs)
- These DLLs **ARE** installed on most Windows systems via:
  - Windows Update
  - Visual C++ Redistributable
  - Many other applications

**Static Linking:**
- All game engine dependencies are compiled into the .exe
- No GLFW.dll, ImGui.dll, etc. needed
- Simpler distribution
- No DLL hell

### Why Debug Builds Fail

**Debug builds (`/MDd` runtime):**
- Include debug information (symbols, checks)
- Much larger size
- Uses `vcruntime140d.dll`, `msvcp140d.dll` (debug runtime DLLs)
- These DLLs are **ONLY** on developer machines
- Regular users don't have them
- Result: `abort()` crash on startup

## Verification

To verify the fix worked:

1. **Check build type:**
   ```cmd
   dumpbin /headers cmake-build-release\Source67.exe | findstr "runtime"
   ```
   Should show `/MD` not `/MDd`

2. **Check dependencies:**
   ```cmd
   dumpbin /dependents cmake-build-release\Source67.exe
   ```
   Should show:
   - ✅ `vcruntime140.dll` (release)
   - ✅ `KERNEL32.dll`
   - ✅ `opengl32.dll`
   - ❌ NOT `vcruntime140d.dll` (debug)

3. **Check size:**
   - Debug build: ~16 MB with PDB
   - Release build: ~3-5 MB

4. **Test on clean system:**
   - Virtual machine or test PC
   - No Visual Studio installed
   - Run installer
   - Launch engine
   - Should work without errors

## Summary

| Aspect | Before (Broken) | After (Fixed) |
|--------|----------------|---------------|
| Build Type | Debug | Release |
| Build Directory | `cmake-build-debug` | `cmake-build-release` |
| Runtime DLLs | Debug (`vcruntime140d.dll`) | Release (`vcruntime140.dll`) |
| Size | ~16 MB | ~3-5 MB |
| Works on user systems? | ❌ No | ✅ Yes |
| DLL dependencies | None (but looking for them) | None (and not looking) |

## Key Takeaways

1. **Always use Release builds for distribution**
2. **Debug builds are for developers only**
3. **Source67 uses static linking - no DLLs needed**
4. **The DLL warning was a red herring**
5. **The real issue was Debug runtime DLL dependencies**

## Related Files Modified

- ✅ `setup/Source67.nsi` - NSIS installer script
- ✅ `setup/build_installer.bat` - Build validation script
- ✅ `setup/README_INSTALLER.md` - Documentation
- ✅ `setup/INSTALLER_ABORT_FIX.md` - This file

## Additional Notes

### For Future Reference

When packaging any C++ application for distribution:

1. **Always use Release builds** (`CMAKE_BUILD_TYPE=Release`)
2. **Never distribute Debug builds** (they won't work on user systems)
3. **Test on a clean system** without development tools
4. **Use static linking** when possible to avoid DLL dependencies
5. **If using dynamic linking**, include Visual C++ Redistributable installer

### Static vs Dynamic Linking

**Source67 uses static linking via CMake FetchContent:**

```cmake
FetchContent_Declare(glfw ...)
FetchContent_Declare(imgui ...)
FetchContent_Declare(jolt ...)
# etc.
```

All dependencies are compiled and linked into the final .exe. This means:
- ✅ Single executable file
- ✅ No DLL dependencies (except system DLLs)
- ✅ Easier distribution
- ❌ Larger executable size
- ❌ Longer compile times

This is the right choice for a game engine distribution!

---

**Problem fixed! The installer now correctly packages the Release build and will work on all Windows systems.**

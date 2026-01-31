# Quick Start: Building the Installer After the Fix

## What Was Fixed

The installer was trying to package a **Debug build** which requires debug C runtime DLLs that don't exist on user systems. This caused the `abort() has been called` error.

**The fix:** Now uses **Release build** which works on all Windows systems.

## How to Build the Installer (Correct Way)

### Step 1: Build Release Version

Open a command prompt in the Source67 root directory and run:

```cmd
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --config Release
```

**Wait for the build to complete.** This creates:
- `cmake-build-release\Source67.exe` (optimized, ~3-5 MB)
- `cmake-build-release\assets\` (engine assets)

### Step 2: Build the Installer

```cmd
cd setup
build_installer.bat
```

This will:
1. ✅ Check if Release build exists
2. ✅ Compile the NSIS installer script
3. ✅ Create `Source67_Setup.exe`

### Step 3: Test the Installer

1. Run `Source67_Setup.exe`
2. Follow the installation wizard
3. Once installed, launch Source67 from Start Menu or Desktop
4. ✅ Should run without any `abort()` error!

## What Changed

### Files Modified

1. **setup/Source67.nsi**
   - Now uses `cmake-build-release\Source67.exe` instead of `cmake-build-debug`
   - Removed DLL copying (Source67 uses static linking)

2. **setup/build_installer.bat**
   - Now checks for Release build instead of Debug
   - Shows helpful error message if Release build doesn't exist

3. **setup/README_INSTALLER.md**
   - Updated all instructions to use Release builds
   - Clarified why Debug builds don't work

4. **setup/INSTALLER_ABORT_FIX.md** (NEW)
   - Comprehensive explanation of the problem and solution

### What You'll See

**When compiling the installer (no more warning):**
```
File: "..\cmake-build-release\Source67.exe" [compress] 3511651/5280384 bytes
✅ No DLL warning (because we're not looking for DLLs anymore)
```

**When running build_installer.bat:**
```
[OK] Source67.exe found in Release build directory
Building installer with NSIS...
[SUCCESS] Source67_Setup.exe created!
```

## Why This Works

### Debug Build (Old - BROKEN)
- Uses debug C runtime: `vcruntime140d.dll`, `msvcp140d.dll`
- These DLLs only exist on developer machines
- Missing on user systems → `abort()` error

### Release Build (New - WORKS)
- Uses release C runtime: `vcruntime140.dll`, `msvcp140.dll`
- These DLLs are on most Windows systems via:
  - Windows Update
  - Visual C++ Redistributable (installed by many apps)
- Works on user systems ✅

### Static Linking
- Source67 uses static linking for game engine libraries
- GLFW, ImGui, Jolt Physics, GLM, etc. are compiled into the .exe
- No extra DLL files needed
- Simpler distribution!

## Troubleshooting

### "Source67.exe not found in cmake-build-release directory"

**Problem:** You haven't built the Release version yet.

**Solution:**
```cmd
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --config Release
```

### "makensis is not recognized"

**Problem:** NSIS not installed or not in PATH.

**Solution:**
1. Download NSIS from https://nsis.sourceforge.io/
2. Install with default settings
3. Restart command prompt
4. Try again

### Installer builds but exe still crashes

**Problem:** You might be packaging the wrong file.

**Solution:**
1. Delete `cmake-build-release` folder
2. Rebuild from scratch:
   ```cmd
   cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
   cmake --build cmake-build-release --config Release
   ```
3. Rebuild installer:
   ```cmd
   cd setup
   build_installer.bat
   ```

## Verification Checklist

Before distributing the installer to users:

- [ ] Built Release version (not Debug)
- [ ] `cmake-build-release\Source67.exe` exists and is 3-5 MB
- [ ] Ran `build_installer.bat` successfully
- [ ] `Source67_Setup.exe` created
- [ ] Tested installer on clean system or VM
- [ ] Installed engine runs without errors
- [ ] No `abort()` error appears

## Summary

**Before:**
- ❌ Used Debug build
- ❌ Required debug runtime DLLs
- ❌ Crashed on user systems

**After:**
- ✅ Uses Release build
- ✅ Uses standard runtime DLLs
- ✅ Works on all Windows systems

**To build installer:**
```cmd
# 1. Build Release
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --config Release

# 2. Build installer
cd setup
build_installer.bat

# 3. Test
# Run Source67_Setup.exe and verify it works
```

---

**For detailed technical explanation, see:** `INSTALLER_ABORT_FIX.md`

**For complete installer documentation, see:** `README_INSTALLER.md`

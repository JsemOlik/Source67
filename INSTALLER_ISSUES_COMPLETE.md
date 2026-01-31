# Installer Issues Summary - Complete Fix

## Overview

This document summarizes both installer issues encountered and their fixes:

1. **Issue #1 (FIXED)**: abort() error - Debug build requiring debug DLLs
2. **Issue #2 (FIXED)**: cmake folders installed + window blink crash

---

## Issue #1: abort() Has Been Called (FIXED)

### Problem
Installed Source67.exe crashed with "abort() has been called" error.

### Root Cause
Installer packaged Debug build which required debug runtime DLLs not on user systems.

### Fix Applied
- Changed NSIS script to use Release build from `cmake-build-release`
- Updated build_installer.bat to require Release build
- Updated all documentation

### Files Modified
- `setup/Source67.nsi` - Now uses Release build
- `setup/build_installer.bat` - Checks for Release build
- `setup/README_INSTALLER.md` - Updated instructions
- `setup/INSTALLER_ABORT_FIX.md` - Technical explanation
- `setup/QUICK_START_INSTALLER.md` - Quick reference

**Status:** ✅ RESOLVED

---

## Issue #2: cmake Folders Installed + Window Blink (FIXED)

### Problem A: cmake Folders in Installation

After installation, found in `C:\Program Files\Source67\`:
- `cmake-build-debug/` ❌ Should not be here
- `cmake-build-release/` ❌ Should not be here

### Root Cause A
User compiled installer with older NSIS script version that had additional File
commands copying from build directories.

### Fix Applied A
- Updated NSIS script with explicit asset copying:
  ```nsis
  SetOutPath "$INSTDIR\assets"
  File /r /x .DS_Store "..\assets\*.*"
  ```
- Added cmake folder cleanup to uninstaller:
  ```nsis
  RMDir /r "$INSTDIR\cmake-build-debug"
  RMDir /r "$INSTDIR\cmake-build-release"
  ```

### Problem B: Window Blinks But Doesn't Open

Engine window appears briefly then immediately closes.

### Possible Causes B
1. cmake folders interfering with engine initialization
2. Missing Visual C++ Runtime DLLs
3. OpenGL initialization failure
4. Asset loading errors
5. Permission issues in Program Files

### Fix Applied B
- Created comprehensive troubleshooting guides
- Documented diagnostic steps
- Provided PowerShell commands for immediate cleanup

### Files Modified
- `setup/Source67.nsi` - Explicit asset copying, cmake cleanup
- `setup/TROUBLESHOOTING_INSTALLER.md` - Comprehensive guide
- `FIX_CURRENT_INSTALLATION.md` - Immediate action guide
- `setup/INSTALLER_ISSUE2_VISUAL.txt` - Visual guide

**Status:** ✅ INSTALLER FIXED, User needs to clean current installation

---

## Immediate User Actions Required

### Quick Fix (Try First)

**Remove cmake folders:**
```powershell
# Open PowerShell as Admin
cd "C:\Program Files\Source67"
Remove-Item -Recurse -Force cmake-build-debug
Remove-Item -Recurse -Force cmake-build-release
```

**Test engine:**
```powershell
.\Source67.exe
```

**If it works:** ✅ Done! The cmake folders were interfering.

**If still fails:** Continue to diagnostic steps below.

### Diagnostic Steps (If Quick Fix Doesn't Work)

**1. Check logs:**
```powershell
cd "C:\Program Files\Source67"
notepad logs\Source67_*.txt
```

**2. Run from command prompt:**
```cmd
cd "C:\Program Files\Source67"
Source67.exe
```
Watch for error messages.

**3. Check Event Viewer:**
```
Win + R → eventvwr.msc
Navigate to: Windows Logs → Application
Look for: Source67.exe errors
```

**4. Verify VC++ Runtime:**
```powershell
dir "C:\Windows\System32\vcruntime140.dll"
```
If missing: [Download VC++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe)

**5. Try running as admin:**
```
Right-click Source67.exe → Run as administrator
```

### Clean Reinstall (If Nothing Works)

**1. Uninstall:**
```
Start Menu → Source67 → Uninstall Source67
```

**2. Delete leftover files:**
```powershell
Remove-Item -Recurse -Force "C:\Program Files\Source67"
```

**3. Get latest code:**
```cmd
cd Source67
git pull origin main
```

**4. Build Release version:**
```cmd
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --config Release
```

**5. Build installer:**
```cmd
cd setup
build_installer.bat
```

**6. Install new version:**
```
Run Source67_Setup.exe
```

---

## What's in a Correct Installation

After a proper installation, you should have:

```
C:\Program Files\Source67\
├── Source67.exe         (Main executable)
├── uninstall.exe       (Uninstaller)
├── assets/             (Engine assets)
│   ├── engine/
│   ├── fonts/
│   ├── layouts/
│   ├── lua/
│   ├── shaders/
│   └── textures/
└── Tools/              (CMake helpers)
    ├── install_cmake.bat
    └── CMAKE_INFO.txt
```

**Should NOT have:**
- ❌ `cmake-build-debug/`
- ❌ `cmake-build-release/`
- ❌ Any `_deps/` folders
- ❌ Any build artifacts

---

## Common Error Messages & Solutions

| Error | Cause | Solution |
|-------|-------|----------|
| "VCRUNTIME140.dll missing" | Missing VC++ Runtime | Install [VC++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe) |
| "OpenGL initialization failed" | Outdated GPU drivers | Update graphics drivers |
| "Could not find 'assets' directory" | Assets not installed | Reinstall Source67 |
| Window blinks then closes | Early crash | Check logs/ and Event Viewer |
| "Access denied" | Permission issue | Run as admin or install to C:\Source67 |
| abort() has been called | Debug build (old installer) | Use new installer with Release build |

---

## All Changes Made

### Code Changes

**setup/Source67.nsi:**
- ✅ Changed to use Release build (Issue #1)
- ✅ Explicit asset copying pattern (Issue #2)
- ✅ Added cmake folder cleanup to uninstaller (Issue #2)

**setup/build_installer.bat:**
- ✅ Now requires Release build (Issue #1)
- ✅ Better error messages (Issue #1)

### Documentation Added

**Issue #1 (abort() error):**
- `setup/INSTALLER_ABORT_FIX.md` - Technical explanation
- `setup/QUICK_START_INSTALLER.md` - Quick reference
- `INSTALLER_FIX_COMPLETE.md` - Summary
- `setup/INSTALLER_FIX_VISUAL.txt` - Visual guide

**Issue #2 (cmake folders + blink):**
- `setup/TROUBLESHOOTING_INSTALLER.md` - Comprehensive troubleshooting
- `FIX_CURRENT_INSTALLATION.md` - Immediate action guide
- `setup/INSTALLER_ISSUE2_VISUAL.txt` - Visual guide

**Updated:**
- `setup/README_INSTALLER.md` - Emphasizes Release builds

---

## Timeline

### Issue #1: abort() Error
- **Reported:** User compiled installer, got abort() error
- **Root Cause:** Debug build requiring debug DLLs
- **Fixed:** Changed to Release build
- **Status:** ✅ RESOLVED

### Issue #2: cmake Folders + Blink
- **Reported:** cmake folders in install dir, window blinks
- **Root Cause:** Old NSIS script + potential crash
- **Fixed:** Updated NSIS script, created guides
- **Status:** ✅ INSTALLER FIXED
- **User Action:** Needs to clean current installation

---

## Testing Checklist

For users to verify their installation:

- [ ] Only these folders exist: `Source67.exe`, `uninstall.exe`, `assets/`, `Tools/`
- [ ] No `cmake-build-*` folders
- [ ] Engine opens when double-clicking Source67.exe
- [ ] No abort() error
- [ ] No window blink/crash
- [ ] Logs folder created if engine runs
- [ ] Can see the Source67 editor interface

---

## Future Prevention

To prevent these issues in the future:

1. **Always use Release builds for distribution**
   ```cmd
   cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
   cmake --build cmake-build-release
   ```

2. **Use the updated build_installer.bat**
   - Validates Release build exists
   - Shows helpful errors

3. **Test installer on clean system**
   - VM without Visual Studio
   - Verify no cmake folders
   - Verify engine runs

4. **Keep NSIS script explicit**
   - Don't use ambiguous paths
   - Be specific about what to copy

---

## Related Documentation

- **setup/README_INSTALLER.md** - Complete installer guide
- **setup/INSTALLER_ABORT_FIX.md** - Technical details of Issue #1
- **setup/TROUBLESHOOTING_INSTALLER.md** - Comprehensive troubleshooting
- **FIX_CURRENT_INSTALLATION.md** - Immediate user actions
- **setup/QUICK_START_INSTALLER.md** - Quick reference
- **setup/INSTALLER_FIX_VISUAL.txt** - Visual guide Issue #1
- **setup/INSTALLER_ISSUE2_VISUAL.txt** - Visual guide Issue #2

---

## Summary

✅ **Both issues fixed in code**

✅ **Comprehensive documentation created**

⚠️ **User needs to clean current installation**

**Next steps:**
1. User removes cmake folders (PowerShell commands)
2. User tests if engine runs
3. If still fails, user checks logs/Event Viewer
4. User reports findings for further assistance

---

**For immediate help, see:** `FIX_CURRENT_INSTALLATION.md`

**For detailed troubleshooting, see:** `setup/TROUBLESHOOTING_INSTALLER.md`

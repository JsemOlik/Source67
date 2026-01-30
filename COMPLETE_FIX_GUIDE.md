# Complete Fix Guide - CMake and Abort() Errors

## Issues Fixed in This Update

### 1. CMake Error: "MSVC_RUNTIME_LIBRARY value not known"
**Root Cause**: BuildSystem.cpp was generating CMakeLists.txt with hardcoded `MSVC_RUNTIME_LIBRARY` property that's not compatible with all CMake/compiler versions.

**Fix Applied**: Removed the problematic line from both:
- `game/CMakeLists.txt.template` (template file)
- `src/Core/BuildSystem.cpp` line 287 (hardcoded fallback template)

### 2. Assets Folder Structure in Installer
**Root Cause**: Installer wasn't preserving the `assets/` folder structure.

**Fix Applied**: Updated `setup/Source67.nsi` to use `File /r "..\assets"` which properly copies the entire assets folder.

---

## What You Need to Do Now

### Step 1: Rebuild the Engine

```bash
# Navigate to Source67 repository
cd C:\Users\olik\Desktop\Coding\Source67

# Clean build (recommended)
rmdir /s /q cmake-build-debug

# Build fresh
cmake -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug --config Debug -j 10
```

### Step 2: Rebuild the Installer

```bash
# Navigate to setup folder
cd C:\Users\olik\Desktop\Coding\Source67\setup

# Right-click Source67.nsi and choose "Compile NSIS Script"
# OR use the batch file:
build_installer.bat
```

### Step 3: Uninstall Old Version

```
1. Go to Control Panel > Programs > Uninstall a program
2. Find "Source67 Engine" and uninstall it
3. OR run: C:\Program Files\Source67\uninstall.exe
```

### Step 4: Install New Version

```
1. Run the newly generated Source67_Setup.exe
2. Follow the installation wizard
3. Install to default location: C:\Program Files\Source67
```

### Step 5: Verify Installation

Check that the folder structure is correct:

```powershell
dir "C:\Program Files\Source67"
```

**Expected output:**
```
Mode                 LastWriteTime         Length Name
----                 -------------         ------ ----
d----                                             assets      <-- This folder must exist!
d----                                             Tools
-a---                                      xxxxxx Source67.exe
-a---                                      xxxxxx uninstall.exe
```

**Inside assets folder:**
```powershell
dir "C:\Program Files\Source67\assets"
```

**Expected:**
```
engine/
fonts/
layouts/
lua/
shaders/
textures/
ui/
```

---

## Fixing Your Game Project

### Remove Old CMakeLists.txt

The old CMakeLists.txt in your Portal project has the problematic `MSVC_RUNTIME_LIBRARY` line.

```powershell
# Delete it
del "C:\Users\olik\Desktop\Portal\game\CMakeLists.txt"
```

### Let BuildSystem Regenerate It

When you build from the editor next time, BuildSystem will automatically create a new, correct CMakeLists.txt.

OR manually test CMake generation:

```powershell
# The engine will auto-generate a correct one
cmake -DCMAKE_BUILD_TYPE=Debug -B "C:\Users\olik\Desktop\Portal\build" -S "C:\Users\olik\Desktop\Portal\game"
```

This should now work WITHOUT the MSVC_RUNTIME_LIBRARY error!

---

## Testing the Build

### Test 1: CMake Configuration Should Succeed

```powershell
cd C:\Users\olik\Desktop\Portal
cmake -DCMAKE_BUILD_TYPE=Debug -B build -S game
```

**Expected output:**
```
-- Selecting Windows SDK version...
-- The CXX compiler identification is MSVC...
CMake Warning at CMakeLists.txt:17 (message):
  No C++ source files found.  Creating minimal Game.dll

-- Game DLL will be built to: C:/Users/olik/Desktop/Portal/build
-- Configuring done (X.Xs)
-- Generating done (X.Xs)    <-- SUCCESS! No MSVC_RUNTIME_LIBRARY error
-- Build files have been written to: C:/Users/olik/Desktop/Portal/build
```

### Test 2: Build Should Succeed

```powershell
cmake --build build --config Debug
```

**Expected:**
```
Building Game...
Game.dll created successfully
```

### Test 3: Check Output

```powershell
dir build\Debug\Game.dll
```

You should see the Game.dll file!

---

## Fixing the Abort() Error

### Possible Causes and Solutions

#### Cause 1: Missing DLL Dependencies (MOST LIKELY)

The installed Source67.exe may be missing runtime DLLs. Check if you have these DLLs in the install folder:

```powershell
dir "C:\Program Files\Source67\*.dll"
```

**If no DLLs are listed**, the installer didn't copy them. This is because:
- There are no DLLs in `cmake-build-debug\` folder
- All dependencies (GLFW, spdlog, etc.) are statically linked

**Solution**: Make sure you built Source67 in **Debug** mode, not Release. Then rebuild the installer.

#### Cause 2: MSVC Runtime Not Installed

Even though we're using static linking, the C++ runtime itself might not be installed on your system.

**Solution**: Install Visual C++ Redistributable:
```
Download: https://aka.ms/vs/17/release/vc_redist.x64.exe
Install it, then try running Source67.exe again
```

#### Cause 3: Assets Folder Missing (FIXED)

This was the previous issue - assets folder wasn't at the right location. After rebuilding the installer, this should be fixed.

#### Cause 4: Running from Wrong Location

Don't try to run Source67.exe from the build folder directly. Always use the installed version:

```powershell
# WRONG (may crash):
C:\Users\olik\Desktop\Coding\Source67\cmake-build-debug\Source67.exe

# CORRECT:
"C:\Program Files\Source67\Source67.exe"
```

---

## Troubleshooting Tips

### Get More Details About the Abort

Try running from command line to see error messages:

```powershell
cd "C:\Program Files\Source67"
.\Source67.exe
```

Look for any error messages before the abort() dialog.

### Check Event Viewer

```
1. Press Win+R
2. Type: eventvwr
3. Go to: Windows Logs > Application
4. Look for recent "Error" entries from Source67.exe
5. Double-click to see details
```

This might tell you which DLL is missing or what went wrong.

### Try Debug Build from Source

If the installed version crashes, try running directly from your build:

```powershell
cd C:\Users\olik\Desktop\Coding\Source67\cmake-build-debug
.\Source67.exe
```

If this works but the installed version doesn't, it's a DLL dependency issue.

### Use Dependency Walker

Download Dependencies.exe:
```
https://github.com/lucasg/Dependencies
```

Run it and open Source67.exe - it will show you which DLLs are missing.

---

## Summary Checklist

- [ ] Rebuild Source67 engine from source (clean build recommended)
- [ ] Rebuild installer (right-click .nsi file)
- [ ] Uninstall old version completely
- [ ] Install new version
- [ ] Verify assets folder structure is correct
- [ ] Delete old CMakeLists.txt from game project
- [ ] Test CMake configuration (should work without errors now)
- [ ] Test Game.dll build (should succeed)
- [ ] Run installed Source67.exe
- [ ] If abort() persists, check DLLs and install VC++ Redistributable

---

## Quick Test Commands

```powershell
# 1. Verify installation structure
dir "C:\Program Files\Source67"
dir "C:\Program Files\Source67\assets"

# 2. Clean your game project
del "C:\Users\olik\Desktop\Portal\game\CMakeLists.txt"
rmdir /s /q "C:\Users\olik\Desktop\Portal\build"

# 3. Test CMake (should work now!)
cd C:\Users\olik\Desktop\Portal
cmake -DCMAKE_BUILD_TYPE=Debug -B build -S game

# 4. Build Game.dll
cmake --build build --config Debug

# 5. Verify output
dir build\Debug\Game.dll
```

---

## Need More Help?

If you still encounter issues after following this guide:

1. **CMake Error Persists**: Make sure you deleted the old CMakeLists.txt and let the system regenerate it
2. **Abort() Error Persists**: Run Dependency Walker to find missing DLLs, or check Event Viewer for details
3. **Build Errors**: Try a completely clean build of the engine itself

Good luck! The CMake error should be completely resolved now. The abort() error is likely a DLL dependency issue that can be resolved by installing Visual C++ Redistributable.

# Troubleshooting Installer Issues

## Problem: cmake-build-* folders in installation directory

### Symptoms
After installing Source67, you see these folders in `C:\Program Files\Source67\`:
- `cmake-build-debug`
- `cmake-build-release`

These should NOT be there!

### Cause
You compiled the installer with an older version of the NSIS script that incorrectly copied build directories.

### Solution

**Option 1: Clean Reinstall (Recommended)**

1. Uninstall Source67 completely:
   ```
   Start Menu → Source67 → Uninstall Source67
   ```

2. Manually delete the installation folder if it still exists:
   ```powershell
   Remove-Item -Recurse -Force "C:\Program Files\Source67"
   ```

3. Rebuild the installer with the updated script:
   ```cmd
   cd Source67\setup
   build_installer.bat
   ```

4. Reinstall Source67 with the new installer

**Option 2: Manual Cleanup**

If you want to keep the current installation:

1. Open PowerShell as Administrator
2. Navigate to the installation directory:
   ```powershell
   cd "C:\Program Files\Source67"
   ```

3. Delete the cmake folders:
   ```powershell
   Remove-Item -Recurse -Force cmake-build-debug
   Remove-Item -Recurse -Force cmake-build-release
   ```

### What Should Be in the Installation Directory

After a correct installation, you should see:

```
C:\Program Files\Source67\
├── Source67.exe         (The engine executable)
├── uninstall.exe       (Uninstaller)
├── assets/             (Engine assets)
│   ├── engine/
│   ├── fonts/
│   ├── layouts/
│   ├── lua/
│   ├── shaders/
│   └── textures/
└── Tools/              (CMake installation helpers)
    ├── install_cmake.bat
    └── CMAKE_INFO.txt
```

**No cmake-build-* folders should exist!**

---

## Problem: Window blinks but engine doesn't open

### Symptoms
- You double-click Source67.exe
- A window appears briefly (blinks)
- The window immediately closes
- The engine doesn't actually open

### Possible Causes

#### 1. Assets Not Found
The engine looks for an `assets/` folder relative to the executable.

**Check:**
```powershell
cd "C:\Program Files\Source67"
dir assets
```

You should see subdirectories: `engine`, `fonts`, `layouts`, `lua`, `shaders`, `textures`

**If assets is missing:**
- Reinstall Source67 completely
- Or manually copy the `assets/` folder from the source repository

#### 2. Application Crash During Initialization

**Check the logs:**
```powershell
cd "C:\Program Files\Source67"
dir logs
```

If a `logs/` folder exists, open the most recent log file:
```powershell
notepad logs\Source67_*.txt
```

Look for error messages that indicate what went wrong.

**Common errors:**
- Missing shader files → Check `assets/shaders/`
- Missing texture files → Check `assets/textures/`
- OpenGL initialization failure → Update graphics drivers

#### 3. Missing Visual C++ Runtime

Even Release builds need the Visual C++ Redistributable runtime.

**Download and install:**
- [Visual C++ Redistributable (x64)](https://aka.ms/vs/17/release/vc_redist.x64.exe)

**Verify installation:**
```powershell
dir "C:\Windows\System32\vcruntime*.dll"
```

You should see files like:
- `vcruntime140.dll`
- `msvcp140.dll`

#### 4. Antivirus or Windows Defender Blocking

**Check Windows Defender:**
1. Open Windows Security
2. Go to "Virus & threat protection"
3. Check "Protection history"
4. Look for any blocks related to Source67.exe

**Add exception if needed:**
1. Windows Security → Virus & threat protection
2. Manage settings → Exclusions
3. Add folder: `C:\Program Files\Source67`

#### 5. Permissions Issue

Running from Program Files may require admin rights.

**Try running as administrator:**
1. Right-click `Source67.exe`
2. Select "Run as administrator"

**Or install to a different location:**
- During installation, choose `C:\Source67\` instead of `C:\Program Files\Source67\`

### Debug Steps

**Step 1: Test from Command Prompt**

This allows you to see any error messages:

```cmd
cd "C:\Program Files\Source67"
Source67.exe
```

Look for any error messages in the console window before it closes.

**Step 2: Test Release Build Directly**

Before reinstalling, test if the Release build works from the build directory:

```cmd
cd Source67\cmake-build-release
Source67.exe
```

If this works but the installed version doesn't, it's an installation issue.

**Step 3: Check Event Viewer**

Windows logs application crashes:

1. Open Event Viewer (`eventvwr.msc`)
2. Navigate to: Windows Logs → Application
3. Look for Error events related to Source67.exe
4. Check the error details for crash information

**Step 4: Use Dependency Walker (Advanced)**

Download [Dependencies](https://github.com/lucasg/Dependencies) to check for missing DLLs:

```cmd
Dependencies.exe "C:\Program Files\Source67\Source67.exe"
```

This will show all DLL dependencies and highlight any that are missing.

---

## Problem: Installer warns about missing DLLs

### Symptom
During NSIS compilation, you see:
```
warning 7010: File: "..\cmake-build-release\*.dll" -> no files found.
```

### Explanation
**This is NOT an error!** It's expected.

Source67 uses **static linking** for all dependencies:
- GLFW
- ImGui
- Jolt Physics
- GLM
- spdlog
- GLAD
- nlohmann/json
- sol2 + Lua

All these libraries are compiled directly into `Source67.exe`, so no external DLL files are needed.

The NSIS script has this line to copy DLLs if they exist, but with static linking, they don't exist (and don't need to).

### Action Required
**None!** This warning is harmless and expected. You can safely ignore it.

---

## Getting More Help

If you're still having issues:

1. **Check the logs** in `C:\Program Files\Source67\logs\`
2. **Run from command prompt** to see error messages
3. **Check Event Viewer** for crash details
4. **Verify Release build** works from cmake-build-release first
5. **Report the issue** with:
   - Log files
   - Error messages from command prompt
   - Event Viewer crash details
   - Your Windows version
   - Graphics card info

---

## Quick Checklist

After installation, verify:

- [ ] Only `Source67.exe`, `uninstall.exe`, `assets/`, and `Tools/` exist
- [ ] No `cmake-build-*` folders
- [ ] `assets/` has subdirectories: engine, fonts, layouts, lua, shaders, textures
- [ ] Running Source67.exe opens a window (doesn't just blink)
- [ ] If it crashes, check `logs/` folder for error messages
- [ ] Visual C++ Redistributable is installed
- [ ] Graphics drivers are up to date

---

**For detailed installation instructions, see:** `README_INSTALLER.md`

**For the installer fix explanation, see:** `INSTALLER_ABORT_FIX.md`

# How to Fix Your Current Installation

## Your Current Situation

You have Source67 installed at `C:\Program Files\Source67\` with these folders:
```
C:\Program Files\Source67\
├── Source67.exe
├── uninstall.exe
├── assets/
├── cmake-build-debug/      ← Should NOT be here!
├── cmake-build-release/    ← Should NOT be here!
└── Tools/
```

The `cmake-build-debug` and `cmake-build-release` folders should NOT be in the installation directory.

Additionally, the engine window blinks but doesn't actually open.

---

## Immediate Fix: Remove cmake Folders

**Option 1: Use PowerShell (Quick)**

1. Open PowerShell as Administrator:
   - Press `Win + X`
   - Select "Windows PowerShell (Admin)" or "Terminal (Admin)"

2. Run these commands:
   ```powershell
   cd "C:\Program Files\Source67"
   Remove-Item -Recurse -Force cmake-build-debug
   Remove-Item -Recurse -Force cmake-build-release
   ```

3. Verify they're gone:
   ```powershell
   dir
   ```

You should now only see:
- `Source67.exe`
- `uninstall.exe`
- `assets`
- `Tools`

**Option 2: Use File Explorer (Manual)**

1. Open File Explorer as Administrator:
   - Right-click File Explorer
   - Select "Run as administrator"

2. Navigate to: `C:\Program Files\Source67\`

3. Select `cmake-build-debug` and `cmake-build-release` folders

4. Press `Shift + Delete` to permanently delete them

5. Confirm deletion

---

## Test if Engine Runs Now

After removing the cmake folders, try running the engine:

```powershell
cd "C:\Program Files\Source67"
.\Source67.exe
```

**Does it work now?**

- ✅ **YES** → Great! The cmake folders were interfering. You're all set!
- ❌ **NO** → Continue to troubleshooting below.

---

## If Engine Still Doesn't Run

### Step 1: Check for Logs

The engine writes logs to help diagnose issues:

```powershell
cd "C:\Program Files\Source67"
dir logs
```

**If logs folder exists:**
```powershell
# View the most recent log
notepad (Get-ChildItem logs\*.txt | Sort-Object LastWriteTime -Descending | Select-Object -First 1).FullName
```

**Look for error messages** that indicate what went wrong.

**If no logs folder:**
The engine is crashing before it can even create logs. This suggests a very early crash.

### Step 2: Run from Command Prompt (See Errors)

Running from command prompt lets you see error messages:

```cmd
cd "C:\Program Files\Source67"
Source67.exe
```

**Watch for:**
- Error messages before the window closes
- Crash dialogs
- System error messages

**Common error messages and fixes:**

| Error Message | Solution |
|---------------|----------|
| "VCRUNTIME140.dll missing" | Install [VC++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe) |
| "OpenGL initialization failed" | Update graphics drivers |
| "Could not find 'assets' directory" | Reinstall Source67 |
| Access denied | Run as administrator |

### Step 3: Check Event Viewer (Detailed Crash Info)

Windows logs all application crashes:

1. Press `Win + R`
2. Type `eventvwr.msc` and press Enter
3. Navigate to: **Windows Logs** → **Application**
4. Look for **Error** events with source "Application Error"
5. Find events related to `Source67.exe`
6. Double-click to see crash details

**Look for:**
- Exception code (e.g., 0xC0000005 = Access Violation)
- Faulting module name (which DLL crashed)
- Crash address

### Step 4: Verify Visual C++ Runtime

Even Release builds need the Visual C++ Redistributable:

**Check if installed:**
```powershell
Get-ItemProperty HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall\* | Where-Object { $_.DisplayName -like "*Visual C++*" } | Select-Object DisplayName, DisplayVersion
```

**If not listed, download and install:**
- [Visual C++ Redistributable x64](https://aka.ms/vs/17/release/vc_redist.x64.exe)

**After installing, restart your computer** and try again.

### Step 5: Try Running as Administrator

Program Files requires admin rights:

1. Right-click `Source67.exe`
2. Select "Run as administrator"
3. Click "Yes" on UAC prompt

**Does it work now?**

---

## Clean Reinstall (If Nothing Works)

If the engine still won't run, do a complete reinstall:

### 1. Uninstall Current Version

**Option A: Use Uninstaller**
```
Start Menu → Source67 → Uninstall Source67
```

**Option B: PowerShell**
```powershell
cd "C:\Program Files\Source67"
.\uninstall.exe
```

### 2. Manually Delete Leftover Files

```powershell
Remove-Item -Recurse -Force "C:\Program Files\Source67"
```

### 3. Rebuild Installer with Latest Code

**In the Source67 source repository:**

```cmd
# Pull latest changes (includes the fix)
git pull origin main

# Build Release version
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --config Release

# Build installer
cd setup
build_installer.bat
```

This creates `Source67_Setup.exe` with the fix.

### 4. Install with New Installer

1. Run `Source67_Setup.exe`
2. Follow installation wizard
3. Choose installation location (consider `C:\Source67\` to avoid admin issues)
4. Complete installation

### 5. Verify Installation

```powershell
cd "C:\Program Files\Source67"  # or wherever you installed
dir
```

**Should see:**
```
Source67.exe
uninstall.exe
assets\
Tools\
```

**Should NOT see:**
```
cmake-build-debug\    ← NO!
cmake-build-release\  ← NO!
```

### 6. Test Engine

```powershell
.\Source67.exe
```

---

## Alternative: Install to Different Location

If you keep having admin/permission issues with Program Files:

1. Uninstall current version
2. Reinstall, but during installation choose:
   - **Custom installation path:** `C:\Source67\`
   - This avoids Program Files admin restrictions

---

## Summary of Fixes Applied

The updated installer (after `git pull`) includes:

1. ✅ **Explicit asset copying** - Won't pick up cmake folders
2. ✅ **cmake folder cleanup** - Uninstaller removes them if present
3. ✅ **Uses Release build** - No debug DLL dependencies
4. ✅ **Comprehensive docs** - Troubleshooting guides

---

## Quick Action Plan

**Right now, do this:**

1. **Remove cmake folders:**
   ```powershell
   cd "C:\Program Files\Source67"
   Remove-Item -Recurse -Force cmake-build-debug, cmake-build-release
   ```

2. **Test engine:**
   ```powershell
   .\Source67.exe
   ```

3. **If still fails, check logs:**
   ```powershell
   notepad logs\Source67_*.txt
   ```

4. **If no logs, check Event Viewer:**
   ```
   eventvwr.msc → Windows Logs → Application
   ```

5. **If still stuck, reinstall:**
   ```
   git pull
   cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
   cmake --build cmake-build-release
   cd setup
   build_installer.bat
   ```

---

## Need More Help?

If the engine still won't run after trying all of the above:

**Provide these details:**

1. **Log file contents** (from `logs/` folder)
2. **Event Viewer crash info** (exception code, faulting module)
3. **Console error messages** (when running from cmd)
4. **Your system info:**
   - Windows version (run `winver`)
   - Graphics card (run `dxdiag`)
   - Installed VC++ redistributables

**Share this info** in a GitHub issue for further assistance.

---

**Next steps:** See `TROUBLESHOOTING_INSTALLER.md` for detailed debugging steps.

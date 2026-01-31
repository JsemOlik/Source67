# How to Test the Program Files Crash Fix

## Your Current Situation

You reported:
- ✅ Engine works from build directory: `C:\Users\olik\Desktop\Coding\Source67\cmake-build-release\Source67.exe`
- ❌ Engine crashes from Program Files: `C:\Program Files\Source67\Source67.exe`
- ❌ Event Viewer shows 0xc0000409 error in ucrtbase.dll

## What Was Fixed

The engine was trying to create a `logs/` directory in Program Files, which requires admin rights. It failed silently, then crashed.

**Fix:** Engine now uses `%LOCALAPPDATA%\Source67\logs\` (your AppData folder) which is user-writable.

## How to Apply the Fix

### Step 1: Update Your Code

```cmd
cd C:\Users\olik\Desktop\Coding\Source67
git pull origin copilot/fix-debug-error-source67
```

### Step 2: Rebuild Release Version

```cmd
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --config Release
```

### Step 3: Rebuild Installer

```cmd
cd setup
build_installer.bat
```

This creates a new `Source67_Setup.exe` with the fix.

### Step 4: Uninstall Old Version

```
Start Menu → Source67 → Uninstall Source67
```

### Step 5: Clean Up Old Installation

Open PowerShell as Administrator:
```powershell
Remove-Item -Recurse -Force "C:\Program Files\Source67"
```

### Step 6: Install New Version

Run the new `Source67_Setup.exe` you just built.

### Step 7: Test

```powershell
cd "C:\Program Files\Source67"
.\Source67.exe
```

**Expected result:** Engine opens successfully! No crash!

## Where to Find Logs Now

Press `Win + R` and paste:
```
%LOCALAPPDATA%\Source67\logs
```

Or navigate to:
```
C:\Users\olik\AppData\Local\Source67\logs\
```

You should see log files like:
```
Source67_2026-01-31_12-34-56.txt
```

## What You Should See

When the engine starts, the console should show:

```
[INFO] CORE: Logger initialized. Log file: C:\Users\olik\AppData\Local\Source67\logs\Source67_2026-01-31_12-34-56.txt
[INFO] CORE: Found assets at: C:\Program Files\Source67
[INFO] CORE: Initializing Window...
[INFO] CORE: Initializing Renderer...
```

If you see warnings about working directory (this is normal and safe):
```
[WARN] CORE: Could not change working directory to C:\Program Files\Source67: ...
[WARN] CORE: Continuing with current working directory...
```

The engine will still work fine!

## Quick Test (Without Reinstalling)

If you want to test the fix before reinstalling:

1. **Copy the new executable:**
   ```cmd
   copy cmake-build-release\Source67.exe "C:\Program Files\Source67\Source67.exe"
   ```
   
   (Say "Yes" to overwrite)

2. **Run it:**
   ```cmd
   cd "C:\Program Files\Source67"
   .\Source67.exe
   ```

3. **Should work now!**

## Troubleshooting

### Still Crashes?

Check if you have the latest code:
```cmd
cd C:\Users\olik\Desktop\Coding\Source67
git log --oneline -1
```

Should show:
```
e7b839c Fix Program Files crash: Use AppData for logs and add error handling
```

### Can't Find Logs?

Open PowerShell and run:
```powershell
Get-ChildItem -Path "$env:LOCALAPPDATA\Source67" -Recurse
```

This will show all Source67 files in AppData.

### Engine Still Won't Open?

Check Event Viewer for new errors:
```
Win + R → eventvwr.msc
Windows Logs → Application
```

Look for Source67.exe errors and report:
- Exception code
- Faulting module
- Error message

## What's Different Now

### Before (Broken)
```
C:\Program Files\Source67\
├── Source67.exe
├── assets\
├── logs\                    ← Tried to create here (fails!)
│   └── Source67_*.txt       ← Permission denied!
└── Tools\
```

### After (Fixed)
```
C:\Program Files\Source67\
├── Source67.exe
├── assets\
└── Tools\

C:\Users\olik\AppData\Local\Source67\
└── logs\                    ← Created here (works!)
    └── Source67_*.txt       ← User has permissions!
```

## Verification Checklist

After applying the fix:

- [ ] Pulled latest code from copilot/fix-debug-error-source67
- [ ] Rebuilt cmake-build-release
- [ ] Rebuilt installer
- [ ] Uninstalled old version
- [ ] Deleted old Program Files directory
- [ ] Installed new version
- [ ] Engine opens from Program Files
- [ ] No crash, no errors
- [ ] Logs found in AppData\Local\Source67\logs
- [ ] Engine UI loads correctly

## If Everything Works

Great! The 0xc0000409 crash is fixed!

You can now:
- Run Source67 from anywhere
- No need for admin rights
- Logs are in a standard location
- Installation works properly

## Report Back

Please confirm:
1. ✅ Engine opens from Program Files
2. ✅ Logs created in AppData
3. ✅ No Event Viewer errors
4. ✅ Engine UI works correctly

Or if issues persist:
1. ❌ Still crashes with error: [paste error]
2. ❌ Event Viewer shows: [paste details]
3. ❌ Log location: [where did logs end up?]

---

**This fix should resolve your issue completely. The engine will now work from Program Files without any crashes!**

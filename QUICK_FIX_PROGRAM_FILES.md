# Quick Fix Guide - Program Files Crash

## Your Issue

✅ Engine works: `C:\Users\olik\Desktop\Coding\Source67\cmake-build-release\Source67.exe`
❌ Engine crashes: `C:\Program Files\Source67\Source67.exe`
❌ Event Viewer: 0xc0000409 in ucrtbase.dll

## What Was Wrong

The engine tried to create a `logs/` folder in Program Files (requires admin permissions). It failed, then crashed.

## Fix Applied

Engine now saves logs to your AppData folder (user-writable):
```
C:\Users\olik\AppData\Local\Source67\logs\
```

## How to Fix Your Installation

### Quick Commands (Copy/Paste)

Open PowerShell in your Source67 directory:

```powershell
# Step 1: Pull the fix
git pull origin copilot/fix-debug-error-source67

# Step 2: Rebuild Release
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --config Release

# Step 3: Rebuild installer
cd setup
.\build_installer.bat
cd ..
```

Then:
1. **Uninstall** old version (Start Menu → Source67 → Uninstall)
2. **Delete** `C:\Program Files\Source67` (may need admin)
3. **Install** new version (run `setup\Source67_Setup.exe`)
4. **Test**: Run from Program Files - should work!

## Where Are My Logs Now?

Press `Win + R` and paste:
```
%LOCALAPPDATA%\Source67\logs
```

Or navigate to:
```
C:\Users\olik\AppData\Local\Source67\logs\
```

## Expected Result

When you run Source67.exe from Program Files, you should see:

```
[INFO] CORE: Logger initialized. Log file: C:\Users\olik\AppData\Local\Source67\logs\Source67_2026-01-31_12-34-56.txt
[INFO] CORE: Found assets at: C:\Program Files\Source67
[INFO] CORE: Initializing Window...
```

✅ **Engine opens successfully!**

## If It Still Doesn't Work

1. **Check Event Viewer**
   - Press `Win + R`, type `eventvwr.msc`
   - Go to: Windows Logs → Application
   - Look for Source67.exe errors

2. **Check Logs**
   - Press `Win + R`, paste `%LOCALAPPDATA%\Source67\logs`
   - Open the latest `.txt` file
   - Look for error messages

3. **Report Back**
   - Event Viewer error details
   - Log file contents
   - Console output

## What's Different

| Before | After |
|--------|-------|
| Logs in Program Files ❌ | Logs in AppData ✅ |
| Crashes without admin ❌ | Works without admin ✅ |
| Silent failures ❌ | Error messages ✅ |
| No fallbacks ❌ | Multiple fallbacks ✅ |

## Verification

After reinstalling:

- [ ] Engine opens from `C:\Program Files\Source67\Source67.exe`
- [ ] No crash, no errors
- [ ] Logs appear in `%LOCALAPPDATA%\Source67\logs\`
- [ ] Console shows log location
- [ ] Engine UI works correctly

## Summary

✅ **Root cause:** Permission errors in Program Files
✅ **Fix:** Use AppData for logs (user-writable)
✅ **Benefit:** Works without admin privileges
✅ **Status:** Ready to test!

---

**See PROGRAM_FILES_CRASH_FIX.md for technical details**
**See TEST_PROGRAM_FILES_FIX.md for detailed testing guide**
**See COMPLETE_SUMMARY.md for all issues and fixes**

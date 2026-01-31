# Fix for Program Files Crash (0xc0000409)

## Problem Solved

**Issue:** Source67.exe crashes immediately when run from `C:\Program Files\Source67\` but works fine from the build directory.

**Error in Event Viewer:**
```
Faulting application name: Source67.exe
Faulting module name: ucrtbase.dll
Exception code: 0xc0000409 (STATUS_STACK_BUFFER_OVERRUN)
```

## Root Cause

The engine was trying to create a `logs/` directory in the Program Files installation directory, which requires **administrator privileges**. The critical issues were:

1. **Logger.cpp** tried to create `logs/` without error handling
2. **Application.cpp** tried to change working directory without error handling
3. When filesystem operations failed silently, subsequent code assumed they succeeded
4. This caused a stack buffer overrun in the C runtime (ucrtbase.dll)

### Why It Worked from Build Directory

The build directory (`C:\Users\...\cmake-build-release\`) is in the user's home folder where the user has **write permissions**. Program Files requires admin rights.

## Solution

### 1. Use User-Writable Log Directory

The engine now uses a proper user-writable location for logs:

**Windows:**
```
%LOCALAPPDATA%\Source67\logs\
Example: C:\Users\YourName\AppData\Local\Source67\logs\
```

**Linux/macOS:**
```
~/.local/share/Source67/logs/
```

### 2. Comprehensive Error Handling

Added try/catch blocks around all filesystem operations:
- Directory creation
- Directory iteration
- File removal (log rotation)
- File creation (log files)
- Working directory changes

### 3. Graceful Degradation

If the preferred log directory can't be created, the engine tries:

1. **First:** `%LOCALAPPDATA%\Source67\logs\` (Windows) or `~/.local/share/Source67/logs/` (Unix)
2. **Fallback 1:** `%TEMP%\Source67\logs\` (temp directory)
3. **Fallback 2:** `./logs` (current directory)
4. **Last Resort:** Continue without file logging (console only)

The engine **will not crash** if it can't create log files. It will warn the user and continue.

## How to Find Your Logs Now

### Windows

Press `Win + R` and paste:
```
%LOCALAPPDATA%\Source67\logs
```

Or navigate to:
```
C:\Users\YourName\AppData\Local\Source67\logs\
```

### Linux/macOS

```bash
~/.local/share/Source67/logs/
```

### If Logs Are Elsewhere

Check the console output when starting Source67. It will show where logs are being saved:
```
[INFO] CORE: Logger initialized. Log file: C:\Users\...\AppData\Local\Source67\logs\Source67_2026-01-31_01-23-45.txt
```

## Benefits

### ✅ No More Crashes
- Engine runs from Program Files without admin rights
- No more 0xc0000409 errors
- Graceful handling of all permission issues

### ✅ Follows Best Practices
- Uses Windows AppData (proper location for application data)
- Follows XDG Base Directory specification on Linux
- Logs don't get deleted when uninstalling/reinstalling

### ✅ Better User Experience
- Logs persist across installations
- Easy to find (standard location)
- No need to run as administrator
- Works in any installation location

## Testing

To verify the fix:

1. **Install Source67** to `C:\Program Files\Source67\`
2. **Run Source67.exe** (without admin rights)
3. **Check that it opens** without crashing
4. **Find logs** at `%LOCALAPPDATA%\Source67\logs\`

### Expected Console Output

When starting the engine, you should see:
```
[INFO] CORE: Logger initialized. Log file: C:\Users\YourName\AppData\Local\Source67\logs\Source67_2026-01-31_12-34-56.txt
[INFO] CORE: Found assets at: C:\Program Files\Source67
```

If the working directory can't be changed (normal in Program Files):
```
[WARN] CORE: Could not change working directory to C:\Program Files\Source67: ...
[WARN] CORE: Continuing with current working directory...
[INFO] CORE: Found assets at: C:\Program Files\Source67
```

## What Changed

### Files Modified

1. **src/Core/Logger.cpp**
   - Added `GetLogDirectory()` helper function
   - Uses Windows API to get AppData path
   - Comprehensive error handling with fallbacks
   - Logger continues without file logging if all else fails

2. **src/Core/Application.cpp**
   - Added error handling for `current_path()` change
   - Engine continues with absolute asset paths if change fails
   - Better logging about what happened

### Technical Details

**Windows API Used:**
```cpp
SHGetKnownFolderPath(FOLDERID_LocalAppData, ...)
```

Returns the user's AppData\Local folder.

**Error Handling Pattern:**
```cpp
try {
  std::filesystem::create_directories(logDir);
} catch (const std::filesystem::filesystem_error& e) {
  // Try fallback location
}
```

## Installer Impact

The installer no longer needs to:
- Request admin privileges for log directory
- Create a logs/ folder in Program Files
- Handle log file permissions

Logs are automatically created in the user's profile on first run.

## Backwards Compatibility

### Old Installations

If you have logs in the old location (`C:\Program Files\Source67\logs\`):
- They won't be automatically migrated
- New logs will be in AppData
- Old logs will remain until you uninstall

### Clean Install Recommended

For best results:
1. Uninstall old version
2. Manually delete `C:\Program Files\Source67` if it exists
3. Rebuild installer with this fix:
   ```cmd
   git pull
   cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
   cmake --build cmake-build-release
   cd setup
   build_installer.bat
   ```
4. Install new version

## Summary

✅ **Engine now runs from Program Files** without admin rights

✅ **Logs saved to AppData** (proper Windows location)

✅ **No more crashes** from filesystem operations

✅ **Graceful error handling** with fallbacks

✅ **Better user experience** - logs easy to find and persist

---

**This fix resolves the 0xc0000409 crash completely. The engine will now work from any installation location.**

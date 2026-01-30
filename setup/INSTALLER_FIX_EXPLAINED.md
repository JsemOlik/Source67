# Installer Fix Explained

## Problem
The installer was showing "Source67.exe not found in cmake-build-debug. Please build the engine first!" even though:
1. The file clearly existed
2. The NSIS compilation log showed it was being included
3. The file was successfully compressed into the installer

## Root Cause
The issue was mixing **runtime checks** with **compile-time file inclusion** in NSIS:

### What Was Wrong (Before)
```nsis
; Runtime check (when installer RUNS)
IfFileExists "..\cmake-build-debug\Debug\Source67.exe" 0 +3
    File "..\cmake-build-debug\Debug\Source67.exe"  ; Compile-time (packages file)
    Goto DoneExe
; If runtime check fails, show error
MessageBox "Source67.exe not found..."
Abort
```

The problem:
- `IfFileExists` = **Runtime** check (when user runs the installer)
- `File` command = **Compile-time** operation (when NSIS builds the installer)
- The `MessageBox` and `Abort` were ALWAYS compiled into the installer
- If the runtime path didn't exist (because installer was moved), it would show the error EVEN THOUGH the file was already packaged inside the installer

### What's Fixed (After)
```nsis
; Compile-time check (when NSIS compiles the installer)
!if /FileExists "..\cmake-build-debug\Debug\Source67.exe"
    File "..\cmake-build-debug\Debug\Source67.exe"
!else if /FileExists "..\cmake-build-debug\Release\Source67.exe"
    File "..\cmake-build-debug\Release\Source67.exe"
!else
    !error "Source67.exe not found! Build the engine first."
!endif
```

The fix:
- `!if` with `/FileExists` = **Compile-time** check
- `!error` = **Compile-time** error (stops NSIS compilation)
- No runtime checks = installer just extracts files (already validated at compile-time)
- Installer can be moved anywhere and will work

## Key Difference: Runtime vs Compile-Time

| NSIS Command | When It Runs | Purpose |
|---|---|---|
| `IfFileExists` | Runtime (during installation) | Check if file exists on user's system |
| `!if /FileExists` | Compile-time (during NSIS build) | Check if file exists before packaging |
| `File` | Compile-time (packages file into installer) | Add file to installer archive |
| `MessageBox` | Runtime (during installation) | Show message to user |
| `!error` | Compile-time (stops compilation) | Prevent building broken installer |

## Testing
After this fix:
1. Compile the installer with `makensis Source67.nsi` or `build_installer.bat`
2. If Source67.exe is missing, you'll get a compile error (good!)
3. If Source67.exe exists, it's packaged and the installer will extract it (good!)
4. Move the installer anywhere and run it - it will work (good!)
5. No more false "file not found" errors (fixed!)

## Additional Notes
- DLL files still use runtime checks with `/nonfatal` flag - this is OK because they're optional
- The main executable must exist at compile-time or the installer won't build
- `build_installer.bat` also validates the build exists before calling NSIS

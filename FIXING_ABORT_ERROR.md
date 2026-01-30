# Fixing the "abort() has been called" Error

## Problem

When running the installed Source67.exe from `C:\Program Files\Source67\`, you get:
```
Debug Error!
Program: C:\Program Files\Source67\Source67.exe
abort() has been called
(Press Retry to debug the application)
```

## Root Causes

This error occurs when the engine fails during initialization. Common causes:

### 1. Missing Assets
The engine expects certain critical assets to exist:
- `assets/fonts/Roboto-Medium.ttf` - Default font
- `assets/shaders/*.glsl` - Shader files
- `assets/engine/app_icon.ico` - Application icon

**Solution**: The installer SHOULD copy these, but verify they exist in:
```
C:\Program Files\Source67\assets\
```

### 2. Missing DLL Dependencies
On Windows with MSVC, the engine may need runtime DLLs.

**Check what DLLs exist** in your build directory:
```
C:\Users\olik\Desktop\Coding\Source67\cmake-build-debug\
```

Look for:
- `spdlogd.dll` (Debug) or `spdlog.dll` (Release)
- Any other `.dll` files

**Solution**: The installer should copy these, but you can manually copy them to:
```
C:\Program Files\Source67\
```

### 3. Working Directory Issues
The engine expects to be run from a directory where it can find `assets/`.

**Current Setup**:
- Executable: `C:\Program Files\Source67\Source67.exe`
- Assets: `C:\Program Files\Source67\assets\`

This SHOULD work if the shortcut is configured correctly.

### 4. ImGui.ini / Settings File
The engine may try to write to `imgui.ini` or settings files but lacks write permission in `Program Files`.

**Solution**: Configure the engine to write user files to AppData:
```cpp
// In Application.cpp, set ImGui.ini path to user directory
std::filesystem::path userDir = std::filesystem::path(getenv("APPDATA")) / "Source67";
std::filesystem::create_directories(userDir);
ImGui::GetIO().IniFilename = (userDir / "imgui.ini").string().c_str();
```

## Immediate Workarounds

### Option 1: Run from Build Directory
Instead of using the installer, run directly from your build:
```
C:\Users\olik\Desktop\Coding\Source67\cmake-build-debug\Source67.exe
```

This works because the assets are in the right relative location.

### Option 2: Copy Build to Different Location
1. Create folder: `C:\Source67\`
2. Copy from `cmake-build-debug\`:
   - `Source67.exe`
   - All `.dll` files
3. Copy from repository root:
   - `assets\` folder (entire directory)
4. Run `C:\Source67\Source67.exe`

### Option 3: Install to Non-Protected Directory
Modify the installer to install to:
```
C:\Source67\
```
Instead of:
```
C:\Program Files\Source67\
```

This avoids Windows UAC/permissions issues.

## Long-Term Fix

We need to update the engine to:

1. **Use proper paths** - Store user data in `%APPDATA%\Source67`
2. **Validate assets** - Check if assets exist before using them
3. **Better error messages** - Show what's missing instead of abort()
4. **Installer validation** - Verify all files are copied correctly

## Debugging Steps

To find the exact cause:

### Step 1: Run with Debugger
If you have Visual Studio installed:
```
Press "Retry" in the error dialog
```
This will attach the debugger and show you exactly where abort() was called.

### Step 2: Check Event Viewer
Windows Event Viewer might have more details:
1. Open Event Viewer
2. Windows Logs → Application
3. Look for Source67.exe errors

### Step 3: Enable Console Output
Modify the engine to keep the console window open:
- In `main.cpp`, remove `FreeConsole()` calls
- Run again and check console output

### Step 4: Use Dependency Walker
Download [Dependencies](https://github.com/lucasg/Dependencies) and analyze:
```
C:\Program Files\Source67\Source67.exe
```
This will show you any missing DLL dependencies.

## Testing the Fix

After applying fixes, test:

1. **Fresh Install Test**
   - Uninstall Source67 completely
   - Delete `C:\Program Files\Source67`
   - Reinstall
   - Run from Start Menu shortcut

2. **Asset Verification**
   ```
   dir "C:\Program Files\Source67\assets" /s
   ```
   Should show all shader, font, texture files

3. **DLL Check**
   ```
   dir "C:\Program Files\Source67\*.dll"
   ```
   Should show any required DLLs

## Contact
If none of these work, provide:
- Output of `dir "C:\Program Files\Source67" /s`
- Contents of Event Viewer
- Debugger stack trace if available

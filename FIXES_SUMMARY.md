# Fixes Summary - Build System Integration

## Issues Fixed

### 1. CMake Build Error: "MSVC_RUNTIME_LIBRARY value not known"

**Problem**: The auto-generated `game/CMakeLists.txt` was using the `MSVC_RUNTIME_LIBRARY` property which requires CMake 3.15+ and may not be recognized by all MSVC compiler versions.

**Solution**: Updated `game/CMakeLists.txt.template` to:
- Check CMake version before using modern property
- Fall back to legacy `/MD` and `/MDd` compiler flags for older CMake versions
- Ensures compatibility across different development environments

**Test**: Run this command in your project:
```bash
cmake -DCMAKE_BUILD_TYPE=Debug -B "C:\Users\YourName\Desktop\YourProject\build" -S "C:\Users\YourName\Desktop\YourProject\game"
```

It should now configure successfully without the "value not known" error.

### 2. abort() Error After Installing Engine

**Problem**: The installer was placing assets incorrectly - they were being installed to:
```
C:\Program Files\Source67\engine\
C:\Program Files\Source67\fonts\
C:\Program Files\Source67\shaders\
```

Instead of the expected:
```
C:\Program Files\Source67\assets\engine\
C:\Program Files\Source67\assets\fonts\
C:\Program Files\Source67\assets\shaders\
```

This caused the engine to fail during initialization when it couldn't find the assets folder, resulting in an abort() call.

**Solution**: Updated `setup/Source67.nsi` to:
- Properly use `/r` (recursive) flag with `File` command
- Exclude `.DS_Store` macOS metadata files
- Preserve the `assets/` folder structure during installation

**Test**: After rebuilding and running the installer:
```powershell
PS> dir "C:\Program Files\Source67\assets"
```

You should see subdirectories: `engine`, `fonts`, `layouts`, `lua`, `shaders`, `textures`, `ui`

### 3. Missing CMakeLists.txt in Game Directory

**Problem**: New game projects didn't have a CMakeLists.txt file, causing build failures.

**Solution**: Added auto-generation feature to BuildSystem:
- `BuildSystem::EnsureGameCMakeLists()` creates CMakeLists.txt if missing
- Uses the template from `game/CMakeLists.txt.template`
- Called automatically before building Game.dll

**Test**: The BuildSystem will now create the file automatically when you click "Build Game" in the editor.

## How to Test All Fixes

### Test 1: Rebuild and Reinstall Engine

1. **Build the engine**:
   ```bash
   cd C:\Users\olik\Desktop\Coding\Source67
   cmake --build cmake-build-debug --target Source67 -j 10
   ```

2. **Build the installer**:
   ```bash
   cd setup
   .\build_installer.bat
   ```

3. **Uninstall old version**:
   - Go to "Add or Remove Programs"
   - Uninstall "Source67 Engine"

4. **Install new version**:
   - Run `setup\Source67_Setup.exe`
   - Install with default options

5. **Verify installation**:
   ```powershell
   dir "C:\Program Files\Source67\assets"
   ```
   You should see all asset subdirectories.

6. **Run the engine**:
   - Launch from Start Menu or Desktop shortcut
   - It should start without the abort() error

### Test 2: Build Game.dll from Separate Project

1. **Create a new project folder**:
   ```powershell
   mkdir C:\Users\olik\Desktop\MyGame
   cd C:\Users\olik\Desktop\MyGame
   ```

2. **Create project structure**:
   ```powershell
   mkdir game
   mkdir assets
   echo '{"name": "MyGame", "version": "1.0.0"}' > .source
   ```

3. **Create a simple game script**:
   ```powershell
   @"
   // MyGame.cpp
   #ifdef _WIN32
   #define EXPORT __declspec(dllexport)
   #else
   #define EXPORT
   #endif

   extern "C" EXPORT const char* GetGameName() {
       return "MyGame";
   }

   extern "C" EXPORT const char* GetGameVersion() {
       return "1.0.0";
   }
   "@ > game\MyGame.cpp
   ```

4. **Open in Source67**:
   - Launch Source67 engine
   - File > Open Project
   - Select `C:\Users\olik\Desktop\MyGame\.source`

5. **Build Game.dll**:
   - Building > Build Game (or press F7)
   - Check console for build output
   - Should succeed and create `build/Game.dll`

6. **Verify output**:
   ```powershell
   dir C:\Users\olik\Desktop\MyGame\build
   ```
   You should see `Game.dll`

## Remaining Known Issues

1. **Missing Assets in Installed Engine**:
   - The installed engine at `C:\Program Files\Source67` needs an `assets/` folder
   - This is now fixed, but requires reinstalling

2. **DLL Dependencies**:
   - Currently no external DLLs are being packaged (warnings during installer build)
   - This is fine for now as Source67.exe is statically linked
   - If future builds use dynamic linking, we'll need to copy those DLLs

## Next Steps

1. Test the installer fix by reinstalling the engine
2. Verify the abort() error is resolved
3. Test building a game project from a separate folder
4. If issues persist, check the console output and share the error messages

## Additional Notes

- **CMake Version**: Requires CMake 3.20+ (comes with Visual Studio or install separately)
- **Compiler**: MSVC 19.50+ (Visual Studio 2022 version 17.10+)
- **Windows SDK**: 10.0.26100.0 or compatible version

## Support

If you encounter issues:
1. Check console output for error messages
2. Verify CMake is installed: `cmake --version`
3. Verify assets folder exists: `dir "C:\Program Files\Source67\assets"`
4. Run from the build directory instead: `cd cmake-build-debug && .\Source67.exe`

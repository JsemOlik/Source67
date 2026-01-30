#include "BuildSystem.h"
#include "Logger.h"
#include "../tools/asset_packer/AssetPacker.h"
#include <iostream>
#include <sstream>
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/wait.h>
#endif

namespace S67 {

// Helper function to check if CMake is available
static bool IsCMakeAvailable() {
#ifdef _WIN32
    int result = system("where cmake >nul 2>nul");
    return result == 0;
#else
    int result = system("which cmake >/dev/null 2>&1");
    return WIFEXITED(result) && WEXITSTATUS(result) == 0;
#endif
}

static void ShowCMakeInstallHelp() {
    S67_CORE_ERROR("========================================");
    S67_CORE_ERROR("CMake is not installed!");
    S67_CORE_ERROR("========================================");
    S67_CORE_ERROR("");
    S67_CORE_ERROR("CMake is required to build Game.dll from C++ source.");
    S67_CORE_ERROR("");
#ifdef _WIN32
    S67_CORE_ERROR("INSTALLATION OPTIONS:");
    S67_CORE_ERROR("");
    S67_CORE_ERROR("1. Run the helper script:");
    S67_CORE_ERROR("   - Open: C:\\Program Files\\Source67\\Tools\\install_cmake.bat");
    S67_CORE_ERROR("");
    S67_CORE_ERROR("2. Download manually:");
    S67_CORE_ERROR("   - Visit: https://cmake.org/download/");
    S67_CORE_ERROR("   - Get: cmake-X.XX.X-windows-x86_64.msi");
    S67_CORE_ERROR("   - Install and add to PATH");
    S67_CORE_ERROR("");
    S67_CORE_ERROR("3. Use package manager:");
    S67_CORE_ERROR("   - Chocolatey: choco install cmake");
    S67_CORE_ERROR("   - Winget: winget install Kitware.CMake");
#else
    S67_CORE_ERROR("Install CMake:");
    S67_CORE_ERROR("  - Ubuntu/Debian: sudo apt install cmake");
    S67_CORE_ERROR("  - macOS: brew install cmake");
    S67_CORE_ERROR("  - Or visit: https://cmake.org/download/");
#endif
    S67_CORE_ERROR("");
    S67_CORE_ERROR("After installation, restart Source67 editor.");
    S67_CORE_ERROR("========================================");
}

BuildSystem::BuildSystem() {
    m_AssetPacker = CreateScope<AssetPacker>();
}

BuildSystem::~BuildSystem() = default;

void BuildSystem::SetConfig(const BuildConfig& config) {
    m_Config = config;
    
    // Set up default build output directory if not specified
    if (m_Config.buildOutputDir.empty() && !m_Config.projectRoot.empty()) {
        m_Config.buildOutputDir = m_Config.projectRoot / "build";
    }
    
    // Configure asset packer
    m_AssetPacker->SetVerbose(m_Config.verbose);
    m_AssetPacker->SetIncludeLua(true);
}

void BuildSystem::Log(const std::string& message) {
    S67_CORE_INFO("[BuildSystem] {}", message);
    if (m_Config.statusCallback) {
        m_Config.statusCallback(message, false);
    }
}

void BuildSystem::LogError(const std::string& message) {
    S67_CORE_ERROR("[BuildSystem] {}", message);
    if (m_Config.statusCallback) {
        m_Config.statusCallback("ERROR: " + message, true);
    }
}

bool BuildSystem::IsProjectValid() const {
    if (m_Config.projectRoot.empty()) {
        return false;
    }
    
    // Check for game directory
    std::filesystem::path gameDir = m_Config.projectRoot / "game";
    if (!std::filesystem::exists(gameDir)) {
        return false;
    }
    
    // Check for assets directory
    std::filesystem::path assetsDir = m_Config.projectRoot / "assets";
    if (!std::filesystem::exists(assetsDir)) {
        return false;
    }
    
    return true;
}

bool BuildSystem::EnsureBuildDirectory() {
    try {
        if (!std::filesystem::exists(m_Config.buildOutputDir)) {
            std::filesystem::create_directories(m_Config.buildOutputDir);
            Log("Created build directory: " + m_Config.buildOutputDir.string());
        }
        return true;
    } catch (const std::exception& e) {
        LogError("Failed to create build directory: " + std::string(e.what()));
        return false;
    }
}

bool BuildSystem::ExecuteCommand(const std::string& command, const std::filesystem::path& workingDir) {
    Log("Executing: " + command);
    Log("Working directory: " + workingDir.string());
    
#ifdef _WIN32
    // Windows: Use system() with working directory change
    std::string fullCommand = "cd /d \"" + workingDir.string() + "\" && " + command;
    int result = system(fullCommand.c_str());
    return result == 0;
#else
    // Unix: Fork and execute
    std::string fullCommand = "cd \"" + workingDir.string() + "\" && " + command;
    int result = system(fullCommand.c_str());
    return WIFEXITED(result) && WEXITSTATUS(result) == 0;
#endif
}

std::filesystem::path BuildSystem::GetGameDLLPath() const {
    std::filesystem::path dllPath = m_Config.buildOutputDir;
    
#ifdef _WIN32
    dllPath /= m_Config.buildType;
    dllPath /= "Game.dll";
#elif defined(__APPLE__)
    dllPath /= "libGame.dylib";
#else
    dllPath /= "libGame.so";
#endif
    
    return dllPath;
}

std::filesystem::path BuildSystem::GetAssetsPackPath() const {
    return m_Config.buildOutputDir / "GameAssets.apak";
}

bool BuildSystem::BuildGame() {
    Log("========================================");
    Log("Building Game.dll...");
    Log("========================================");
    
    // Check if CMake is available
    if (!IsCMakeAvailable()) {
        LogError("CMake is not installed or not in PATH");
        ShowCMakeInstallHelp();
        return false;
    }
    
    if (!IsProjectValid()) {
        LogError("Invalid project structure. Need game/ and assets/ directories in: " + 
                 m_Config.projectRoot.string());
        return false;
    }
    
    if (!EnsureBuildDirectory()) {
        return false;
    }
    
    // Step 1: Configure CMake
    if (!ConfigureGameCMake()) {
        LogError("CMake configuration failed");
        return false;
    }
    
    // Step 2: Compile
    if (!CompileGameDLL()) {
        LogError("Game.dll compilation failed");
        return false;
    }
    
    Log("========================================");
    Log("Game.dll built successfully!");
    Log("Output: " + GetGameDLLPath().string());
    Log("========================================");
    
    return true;
}

bool BuildSystem::EnsureGameCMakeLists() {
    std::filesystem::path gameDir = m_Config.projectRoot / "game";
    std::filesystem::path cmakeListsPath = gameDir / "CMakeLists.txt";
    
    // Check if CMakeLists.txt already exists
    if (std::filesystem::exists(cmakeListsPath)) {
        return true; // Already exists, we're good
    }
    
    Log("CMakeLists.txt not found in game directory, creating from template...");
    
    // Try to find the template in the engine directory
    // The template should be in the engine's game/ folder
    std::filesystem::path templatePath;
    
    // Try multiple possible locations for the template
    std::vector<std::filesystem::path> possiblePaths = {
        m_Config.engineRoot / "game" / "CMakeLists.txt.template",
        std::filesystem::current_path() / "game" / "CMakeLists.txt.template",
        gameDir / "CMakeLists.txt.template"
    };
    
    for (const auto& path : possiblePaths) {
        if (std::filesystem::exists(path)) {
            templatePath = path;
            break;
        }
    }
    
    // If we found the template, copy it
    if (!templatePath.empty()) {
        try {
            std::filesystem::copy_file(templatePath, cmakeListsPath);
            Log("Created CMakeLists.txt from template");
            return true;
        } catch (const std::exception& e) {
            LogError("Failed to copy template: " + std::string(e.what()));
        }
    }
    
    // If no template found, generate a basic one
    Log("Generating default CMakeLists.txt...");
    try {
        std::ofstream cmake(cmakeListsPath);
        cmake << "# Game DLL Build Configuration\n";
        cmake << "# Auto-generated by Source67 Engine's BuildSystem\n\n";
        cmake << "cmake_minimum_required(VERSION 3.20)\n";
        cmake << "project(Game VERSION 1.0.0 LANGUAGES CXX)\n\n";
        cmake << "# Set C++20 standard\n";
        cmake << "set(CMAKE_CXX_STANDARD 20)\n";
        cmake << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n";
        cmake << "# Find all C++ source files\n";
        cmake << "file(GLOB_RECURSE GAME_SOURCES \"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp\")\n";
        cmake << "file(GLOB_RECURSE GAME_HEADERS \"${CMAKE_CURRENT_SOURCE_DIR}/*.h\")\n\n";
        cmake << "# Create dummy game if no sources exist\n";
        cmake << "if(NOT GAME_SOURCES)\n";
        cmake << "    message(WARNING \"No C++ source files found. Creating minimal Game.dll\")\n";
        cmake << "    file(WRITE \"${CMAKE_CURRENT_BINARY_DIR}/dummy_game.cpp\" \n";
        cmake << "        \"// Auto-generated minimal game DLL\\n\"\n";
        cmake << "        \"#ifdef _WIN32\\n\"\n";
        cmake << "        \"#define EXPORT __declspec(dllexport)\\n\"\n";
        cmake << "        \"#else\\n\"\n";
        cmake << "        \"#define EXPORT\\n\"\n";
        cmake << "        \"#endif\\n\\n\"\n";
        cmake << "        \"extern \\\"C\\\" EXPORT const char* GetGameName() {\\n\"\n";
        cmake << "        \"    return \\\"MyGame\\\";\\n\"\n";
        cmake << "        \"}\\n\\n\"\n";
        cmake << "        \"extern \\\"C\\\" EXPORT const char* GetGameVersion() {\\n\"\n";
        cmake << "        \"    return \\\"1.0.0\\\";\\n\"\n";
        cmake << "        \"}\\n\"\n";
        cmake << "    )\n";
        cmake << "    set(GAME_SOURCES \"${CMAKE_CURRENT_BINARY_DIR}/dummy_game.cpp\")\n";
        cmake << "endif()\n\n";
        cmake << "# Create Game DLL\n";
        cmake << "add_library(Game SHARED ${GAME_SOURCES} ${GAME_HEADERS})\n\n";
        cmake << "# Set output to build folder\n";
        cmake << "set_target_properties(Game PROPERTIES\n";
        cmake << "    RUNTIME_OUTPUT_DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}/../build\"\n";
        cmake << "    LIBRARY_OUTPUT_DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}/../build\"\n";
        cmake << ")\n\n";
        cmake << "# Windows-specific\n";
        cmake << "if(MSVC)\n";
        cmake << "    target_compile_options(Game PRIVATE /W4)\n";
        cmake << "    set_property(TARGET Game PROPERTY MSVC_RUNTIME_LIBRARY \"MultiThreadedDLL$<$<CONFIG:Debug>:Debug>\")\n";
        cmake << "endif()\n\n";
        cmake << "# Include directories\n";
        cmake << "target_include_directories(Game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})\n\n";
        cmake << "message(STATUS \"Game DLL will be built to: ${CMAKE_CURRENT_SOURCE_DIR}/../build\")\n";
        cmake.close();
        
        Log("Generated default CMakeLists.txt");
        return true;
    } catch (const std::exception& e) {
        LogError("Failed to generate CMakeLists.txt: " + std::string(e.what()));
        return false;
    }
}

bool BuildSystem::ConfigureGameCMake() {
    Log("Configuring CMake for Game.dll...");
    
    // Ensure CMakeLists.txt exists
    if (!EnsureGameCMakeLists()) {
        LogError("Failed to create or find CMakeLists.txt in game directory");
        return false;
    }
    
    std::filesystem::path gameDir = m_Config.projectRoot / "game";
    
    std::ostringstream cmd;
    cmd << "cmake";
    cmd << " -DCMAKE_BUILD_TYPE=" << m_Config.buildType;
    cmd << " -B \"" << m_Config.buildOutputDir.string() << "\"";
    cmd << " -S \"" << gameDir.string() << "\"";
    
    return ExecuteCommand(cmd.str(), gameDir);
}

bool BuildSystem::CompileGameDLL() {
    Log("Compiling Game.dll...");
    
    std::ostringstream cmd;
    cmd << "cmake --build \"" << m_Config.buildOutputDir.string() << "\"";
    cmd << " --config " << m_Config.buildType;
    
    return ExecuteCommand(cmd.str(), m_Config.projectRoot);
}

bool BuildSystem::BuildAssets() {
    Log("========================================");
    Log("Packing GameAssets.apak...");
    Log("========================================");
    
    if (!IsProjectValid()) {
        LogError("Invalid project structure. Need game/ and assets/ directories in: " + 
                 m_Config.projectRoot.string());
        return false;
    }
    
    if (!EnsureBuildDirectory()) {
        return false;
    }
    
    if (!PackAssets()) {
        LogError("Asset packing failed");
        return false;
    }
    
    Log("========================================");
    Log("GameAssets.apak created successfully!");
    Log("Output: " + GetAssetsPackPath().string());
    
    // Print file size
    try {
        auto fileSize = std::filesystem::file_size(GetAssetsPackPath());
        Log("Size: " + std::to_string(fileSize) + " bytes");
    } catch (...) {}
    
    Log("========================================");
    
    return true;
}

bool BuildSystem::PackAssets() {
    Log("Scanning and packing assets...");
    
    std::filesystem::path assetsDir = m_Config.projectRoot / "assets";
    std::filesystem::path outputPath = GetAssetsPackPath();
    
    // Use the AssetPacker to pack assets
    bool success = m_AssetPacker->PackAssets(assetsDir, outputPath);
    
    if (!success) {
        LogError("AssetPacker failed to pack assets");
        return false;
    }
    
    return true;
}

bool BuildSystem::BuildAll() {
    Log("========================================");
    Log("Building ALL (Game.dll + GameAssets.apak)");
    Log("========================================");
    
    // Build game first
    if (!BuildGame()) {
        return false;
    }
    
    Log("");
    
    // Then build assets
    if (!BuildAssets()) {
        return false;
    }
    
    Log("");
    Log("========================================");
    Log("BUILD ALL COMPLETED SUCCESSFULLY!");
    Log("========================================");
    Log("Game.dll: " + GetGameDLLPath().string());
    Log("GameAssets.apak: " + GetAssetsPackPath().string());
    Log("========================================");
    
    return true;
}

bool BuildSystem::CopyEngineExecutable(const std::string& gameName) {
    Log("Copying engine executable...");
    
    // Find the engine executable
    std::filesystem::path engineExe;
    
#ifdef _WIN32
    // Look for Source67.exe in engine root or build directories
    std::vector<std::filesystem::path> searchPaths = {
        m_Config.engineRoot / "cmake-build-debug" / "Debug" / "Source67.exe",
        m_Config.engineRoot / "cmake-build-release" / "Release" / "Source67.exe",
        m_Config.engineRoot / "build" / "Debug" / "Source67.exe",
        m_Config.engineRoot / "build" / "Release" / "Source67.exe",
    };
#else
    std::vector<std::filesystem::path> searchPaths = {
        m_Config.engineRoot / "cmake-build-debug" / "Source67",
        m_Config.engineRoot / "cmake-build-release" / "Source67",
        m_Config.engineRoot / "build" / "Source67",
    };
#endif
    
    for (const auto& path : searchPaths) {
        if (std::filesystem::exists(path)) {
            engineExe = path;
            break;
        }
    }
    
    if (engineExe.empty()) {
        LogError("Could not find Source67 executable in engine directory");
        return false;
    }
    
    // Copy to build directory with game name
    std::filesystem::path destPath = m_Config.buildOutputDir / (gameName + engineExe.extension().string());
    
    try {
        std::filesystem::copy_file(engineExe, destPath, std::filesystem::copy_options::overwrite_existing);
        Log("Copied: " + engineExe.filename().string() + " -> " + destPath.string());
        return true;
    } catch (const std::exception& e) {
        LogError("Failed to copy engine executable: " + std::string(e.what()));
        return false;
    }
}

bool BuildSystem::PackageForDistribution(const std::string& gameName, const std::string& version) {
    Log("========================================");
    Log("Creating distribution package...");
    Log("Package: " + gameName + " v" + version);
    Log("========================================");
    
    if (!IsProjectValid()) {
        LogError("Invalid project structure");
        return false;
    }
    
    // Ensure everything is built first
    if (!BuildAll()) {
        LogError("Build failed, cannot create package");
        return false;
    }
    
    // Create package directory
    std::string packageDirName = gameName + "_v" + version;
    std::filesystem::path packageDir = m_Config.buildOutputDir / packageDirName;
    
    try {
        if (std::filesystem::exists(packageDir)) {
            std::filesystem::remove_all(packageDir);
        }
        std::filesystem::create_directories(packageDir);
    } catch (const std::exception& e) {
        LogError("Failed to create package directory: " + std::string(e.what()));
        return false;
    }
    
    // Copy engine executable (renamed to game name)
    if (!CopyEngineExecutable(gameName)) {
        return false;
    }
    
    // Copy Game.dll
    try {
        std::filesystem::path srcDLL = GetGameDLLPath();
        std::filesystem::path destDLL = packageDir / srcDLL.filename();
        std::filesystem::copy_file(srcDLL, destDLL);
        Log("Copied: Game.dll");
    } catch (const std::exception& e) {
        LogError("Failed to copy Game.dll: " + std::string(e.what()));
        return false;
    }
    
    // Copy GameAssets.apak
    try {
        std::filesystem::path srcPack = GetAssetsPackPath();
        std::filesystem::path destPack = packageDir / "GameAssets.apak";
        std::filesystem::copy_file(srcPack, destPack);
        Log("Copied: GameAssets.apak");
    } catch (const std::exception& e) {
        LogError("Failed to copy GameAssets.apak: " + std::string(e.what()));
        return false;
    }
    
    // Create README
    std::filesystem::path readmePath = packageDir / "README.txt";
    try {
        std::ofstream readme(readmePath);
        readme << "========================================\n";
        readme << "    " << gameName << " v" << version << "\n";
        readme << "========================================\n";
        readme << "\n";
        readme << "HOW TO PLAY:\n";
        readme << "1. Double-click " << gameName << " to start\n";
        readme << "2. Use WASD to move, Mouse to look\n";
        readme << "3. Press ESC to pause/quit\n";
        readme << "\n";
        readme << "Created with Source67 Game Engine\n";
        readme << "========================================\n";
        readme.close();
        Log("Created: README.txt");
    } catch (const std::exception& e) {
        LogError("Failed to create README: " + std::string(e.what()));
    }
    
    Log("========================================");
    Log("PACKAGE CREATED SUCCESSFULLY!");
    Log("Location: " + packageDir.string());
    Log("========================================");
    
    return true;
}

} // namespace S67

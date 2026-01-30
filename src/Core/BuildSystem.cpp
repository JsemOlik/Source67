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

bool BuildSystem::ConfigureGameCMake() {
    Log("Configuring CMake for Game.dll...");
    
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

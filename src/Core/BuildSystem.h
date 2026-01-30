#pragma once

#include "Core/Base.h"
#include <filesystem>
#include <string>
#include <functional>

namespace S67 {

// Forward declarations
class AssetPacker;

// Build status callback
using BuildStatusCallback = std::function<void(const std::string& message, bool isError)>;

// Build configuration
struct BuildConfig {
    std::filesystem::path projectRoot;      // Project root directory (where game/ and assets/ are)
    std::filesystem::path engineRoot;       // Engine root directory (where Source67 is)
    std::filesystem::path buildOutputDir;   // Where to put build outputs (projectRoot/build)
    std::string buildType = "Debug";        // Debug or Release
    bool verbose = false;                   // Verbose logging
    
    BuildStatusCallback statusCallback;     // Callback for status updates
};

// Native build system - builds Game.dll and GameAssets.apak without scripts
class BuildSystem {
public:
    BuildSystem();
    ~BuildSystem();

    // Configuration
    void SetConfig(const BuildConfig& config);
    const BuildConfig& GetConfig() const { return m_Config; }

    // Build operations
    bool BuildGame();                       // Build Game.dll using CMake
    bool BuildAssets();                     // Pack assets using AssetPacker
    bool BuildAll();                        // Build both game and assets
    bool PackageForDistribution(const std::string& gameName, const std::string& version);

    // Utilities
    bool IsProjectValid() const;            // Check if project has required structure
    std::filesystem::path GetGameDLLPath() const;
    std::filesystem::path GetAssetsPackPath() const;

private:
    // Internal build steps
    bool ConfigureGameCMake();
    bool CompileGameDLL();
    bool PackAssets();
    bool CopyEngineExecutable(const std::string& gameName);

    // Helpers
    void Log(const std::string& message);
    void LogError(const std::string& message);
    bool ExecuteCommand(const std::string& command, const std::filesystem::path& workingDir);
    bool EnsureBuildDirectory();

private:
    BuildConfig m_Config;
    Scope<AssetPacker> m_AssetPacker;
};

} // namespace S67

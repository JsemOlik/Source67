#pragma once

#include "AssetPackerTypes.h"
#include <filesystem>
#include <fstream>
#include <memory>

namespace S67 {

class AssetPacker {
public:
    AssetPacker();
    ~AssetPacker();

    // Configuration
    void SetCompressionType(CompressionType type) { m_CompressionType = type; }
    void SetVerbose(bool verbose) { m_Verbose = verbose; }
    void SetIncludeLua(bool include) { m_IncludeLua = include; }
    void SetLuaDirectory(const std::string& dir) { m_LuaDir = dir; }

    // Main operations
    bool PackAssets(const std::filesystem::path& inputDir, const std::filesystem::path& outputFile);
    bool ValidatePack(const std::filesystem::path& packFile);

private:
    // Asset discovery
    void ScanAssets(const std::filesystem::path& inputDir);
    void ScanLuaScripts(const std::filesystem::path& luaDir);
    AssetType DetermineAssetType(const std::filesystem::path& path);

    // Packing operations
    bool WriteHeader(std::ofstream& file);
    bool WriteAssetData(std::ofstream& file);
    bool WriteIndexTable(std::ofstream& file);
    bool WriteLuaScriptIndex(std::ofstream& file);
    bool WriteFooter(std::ofstream& file);

    // Compression
    std::vector<uint8_t> CompressData(const std::vector<uint8_t>& data);

    // Utilities
    void Log(const std::string& message);
    void LogError(const std::string& message);

private:
    std::vector<AssetEntry> m_Assets;
    std::vector<LuaScriptEntry> m_LuaScripts;
    CompressionType m_CompressionType = CompressionType::NONE;
    bool m_Verbose = false;
    bool m_IncludeLua = true;
    std::string m_LuaDir = "lua";
    uint64_t m_CurrentOffset = 0;
};

} // namespace S67

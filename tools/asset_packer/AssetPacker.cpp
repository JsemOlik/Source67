#include "AssetPacker.h"
#include <iostream>
#include <fstream>
#include <algorithm>

namespace S67 {

AssetPacker::AssetPacker() = default;
AssetPacker::~AssetPacker() = default;

void AssetPacker::Log(const std::string& message) {
    if (m_Verbose) {
        std::cout << "[AssetPacker] " << message << std::endl;
    }
}

void AssetPacker::LogError(const std::string& message) {
    std::cerr << "[AssetPacker ERROR] " << message << std::endl;
}

AssetType AssetPacker::DetermineAssetType(const std::filesystem::path& path) {
    auto ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".bmp") {
        return AssetType::ASSET_TEXTURE;
    }
    else if (ext == ".obj" || ext == ".fbx" || ext == ".gltf" || ext == ".glb") {
        return AssetType::ASSET_MODEL;
    }
    else if (ext == ".s67") {
        return AssetType::ASSET_SCENE;
    }
    else if (ext == ".glsl" || ext == ".vert" || ext == ".frag" || ext == ".geom") {
        return AssetType::ASSET_SHADER;
    }
    else if (ext == ".ttf" || ext == ".otf") {
        return AssetType::ASSET_FONT;
    }
    else if (ext == ".lua") {
        return AssetType::ASSET_LUA_SCRIPT;
    }
    else if (ext == ".json") {
        return AssetType::ASSET_CONFIG_JSON;
    }
    else if (ext == ".wav" || ext == ".mp3" || ext == ".ogg") {
        return AssetType::ASSET_AUDIO;
    }
    
    return AssetType::ASSET_UNKNOWN;
}

void AssetPacker::ScanAssets(const std::filesystem::path& inputDir) {
    Log("Scanning assets in: " + inputDir.string());

    for (const auto& entry : std::filesystem::recursive_directory_iterator(inputDir)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        auto path = entry.path();
        auto type = DetermineAssetType(path);

        // Skip Lua scripts if they're in the lua directory (handled separately)
        if (type == AssetType::ASSET_LUA_SCRIPT && m_IncludeLua) {
            auto relPath = std::filesystem::relative(path, inputDir);
            if (relPath.string().find(m_LuaDir) == 0) {
                continue; // Will be handled by ScanLuaScripts
            }
        }

        if (type == AssetType::ASSET_UNKNOWN) {
            Log("Skipping unknown asset type: " + path.string());
            continue;
        }

        // Read file data
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) {
            LogError("Failed to open file: " + path.string());
            continue;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> data(size);
        if (!file.read(reinterpret_cast<char*>(data.data()), size)) {
            LogError("Failed to read file: " + path.string());
            continue;
        }

        // Create asset entry
        AssetEntry asset;
        asset.path = std::filesystem::relative(path, inputDir).string();
        asset.type = type;
        asset.data = std::move(data);
        asset.path_hash = HashString(asset.path);
        asset.checksum = CalculateCRC32(asset.data.data(), asset.data.size());

        m_Assets.push_back(std::move(asset));
        Log("Added asset: " + asset.path + " (" + AssetTypeToString(type) + ")");
    }

    Log("Found " + std::to_string(m_Assets.size()) + " assets");
}

void AssetPacker::ScanLuaScripts(const std::filesystem::path& luaDir) {
    if (!std::filesystem::exists(luaDir)) {
        Log("Lua directory not found: " + luaDir.string());
        return;
    }

    Log("Scanning Lua scripts in: " + luaDir.string());

    for (const auto& entry : std::filesystem::recursive_directory_iterator(luaDir)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        auto path = entry.path();
        if (path.extension() != ".lua") {
            continue;
        }

        // Read file data
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) {
            LogError("Failed to open Lua script: " + path.string());
            continue;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> data(size);
        if (!file.read(reinterpret_cast<char*>(data.data()), size)) {
            LogError("Failed to read Lua script: " + path.string());
            continue;
        }

        // Create Lua script entry
        LuaScriptEntry script;
        script.path = std::filesystem::relative(path, luaDir.parent_path()).string();
        script.data = std::move(data);
        script.path_hash = HashString(script.path);
        script.checksum = CalculateCRC32(script.data.data(), script.data.size());

        m_LuaScripts.push_back(std::move(script));
        Log("Added Lua script: " + script.path);
    }

    Log("Found " + std::to_string(m_LuaScripts.size()) + " Lua scripts");
}

std::vector<uint8_t> AssetPacker::CompressData(const std::vector<uint8_t>& data) {
    // For now, no compression implemented
    // In production, would use zlib (deflate) or lz4
    return data;
}

bool AssetPacker::WriteHeader(std::ofstream& file) {
    AssetPackHeader header = {};
    header.magic = ASSETPACK_MAGIC;
    header.version = ASSETPACK_VERSION;
    header.asset_count = static_cast<uint32_t>(m_Assets.size());
    header.lua_script_count = static_cast<uint32_t>(m_LuaScripts.size());
    header.flags = (m_CompressionType != CompressionType::NONE) ? FLAG_COMPRESSED : FLAG_NONE;
    header.index_offset = 0; // Will be filled later
    
    file.write(reinterpret_cast<const char*>(&header), sizeof(header));
    return file.good();
}

bool AssetPacker::WriteAssetData(std::ofstream& file) {
    m_CurrentOffset = sizeof(AssetPackHeader);

    // Write all asset data
    for (auto& asset : m_Assets) {
        file.write(reinterpret_cast<const char*>(asset.data.data()), asset.data.size());
        m_CurrentOffset += asset.data.size();
    }

    // Write all Lua script data
    for (auto& script : m_LuaScripts) {
        file.write(reinterpret_cast<const char*>(script.data.data()), script.data.size());
        m_CurrentOffset += script.data.size();
    }

    return file.good();
}

bool AssetPacker::WriteIndexTable(std::ofstream& file) {
    uint64_t offset = sizeof(AssetPackHeader);

    for (const auto& asset : m_Assets) {
        AssetIndexEntry entry = {};
        entry.path_hash = asset.path_hash;
        entry.type = asset.type;
        entry.offset = offset;
        entry.size = asset.data.size();
        entry.compressed_size = 0; // No compression for now
        entry.compression = CompressionType::NONE;
        entry.checksum = asset.checksum;
        entry.reserved = 0;

        file.write(reinterpret_cast<const char*>(&entry), sizeof(entry));
        offset += asset.data.size();
    }

    return file.good();
}

bool AssetPacker::WriteLuaScriptIndex(std::ofstream& file) {
    uint64_t offset = sizeof(AssetPackHeader);
    
    // Skip past asset data
    for (const auto& asset : m_Assets) {
        offset += asset.data.size();
    }

    for (const auto& script : m_LuaScripts) {
        LuaScriptIndexEntry entry = {};
        entry.path_hash = script.path_hash;
        entry.offset = offset;
        entry.size = script.data.size();
        entry.checksum = script.checksum;
        entry.reserved = 0;

        file.write(reinterpret_cast<const char*>(&entry), sizeof(entry));
        offset += script.data.size();
    }

    return file.good();
}

bool AssetPacker::WriteFooter(std::ofstream& file) {
    AssetPackFooter footer = {};
    footer.data_checksum = 0; // TODO: Calculate actual checksum
    footer.metadata_checksum = 0; // TODO: Calculate actual checksum

    file.write(reinterpret_cast<const char*>(&footer), sizeof(footer));
    return file.good();
}

bool AssetPacker::PackAssets(const std::filesystem::path& inputDir, const std::filesystem::path& outputFile) {
    Log("Starting asset packing...");
    Log("Input directory: " + inputDir.string());
    Log("Output file: " + outputFile.string());

    // Clear previous data
    m_Assets.clear();
    m_LuaScripts.clear();

    // Scan assets
    ScanAssets(inputDir);

    // Scan Lua scripts if enabled
    if (m_IncludeLua) {
        auto luaPath = inputDir / m_LuaDir;
        ScanLuaScripts(luaPath);
    }

    // Open output file
    std::ofstream file(outputFile, std::ios::binary);
    if (!file) {
        LogError("Failed to create output file: " + outputFile.string());
        return false;
    }

    // Write header (placeholder)
    if (!WriteHeader(file)) {
        LogError("Failed to write header");
        return false;
    }

    // Write asset data
    uint64_t indexOffset = file.tellp();
    if (!WriteAssetData(file)) {
        LogError("Failed to write asset data");
        return false;
    }

    // Store index offset
    indexOffset = file.tellp();

    // Write index table
    if (!WriteIndexTable(file)) {
        LogError("Failed to write index table");
        return false;
    }

    // Write Lua script index
    if (!WriteLuaScriptIndex(file)) {
        LogError("Failed to write Lua script index");
        return false;
    }

    // Write footer
    if (!WriteFooter(file)) {
        LogError("Failed to write footer");
        return false;
    }

    // Update header with correct index offset
    file.seekp(0);
    AssetPackHeader header = {};
    header.magic = ASSETPACK_MAGIC;
    header.version = ASSETPACK_VERSION;
    header.asset_count = static_cast<uint32_t>(m_Assets.size());
    header.lua_script_count = static_cast<uint32_t>(m_LuaScripts.size());
    header.flags = (m_CompressionType != CompressionType::NONE) ? FLAG_COMPRESSED : FLAG_NONE;
    header.index_offset = indexOffset;
    file.write(reinterpret_cast<const char*>(&header), sizeof(header));

    file.close();

    Log("Asset packing complete!");
    Log("  Total assets: " + std::to_string(m_Assets.size()));
    Log("  Lua scripts: " + std::to_string(m_LuaScripts.size()));
    Log("  Output size: " + std::to_string(std::filesystem::file_size(outputFile)) + " bytes");

    return true;
}

bool AssetPacker::ValidatePack(const std::filesystem::path& packFile) {
    Log("Validating asset pack: " + packFile.string());

    std::ifstream file(packFile, std::ios::binary);
    if (!file) {
        LogError("Failed to open pack file");
        return false;
    }

    // Read header
    AssetPackHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (header.magic != ASSETPACK_MAGIC) {
        LogError("Invalid magic number");
        return false;
    }

    if (header.version != ASSETPACK_VERSION) {
        LogError("Unsupported version: " + std::to_string(header.version));
        return false;
    }

    Log("Validation successful!");
    Log("  Version: " + std::to_string(header.version));
    Log("  Assets: " + std::to_string(header.asset_count));
    Log("  Lua scripts: " + std::to_string(header.lua_script_count));

    return true;
}

} // namespace S67

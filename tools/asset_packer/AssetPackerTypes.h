#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace S67 {

// Magic number for Source67 asset pack format
constexpr uint32_t ASSETPACK_MAGIC = 0x36374150; // "AP67" in hex
constexpr uint32_t ASSETPACK_VERSION = 2;

// Asset types supported in the pack
enum class AssetType : uint32_t {
    ASSET_UNKNOWN = 0,
    ASSET_TEXTURE = 1,
    ASSET_MODEL = 2,
    ASSET_SCENE = 3,
    ASSET_SHADER = 4,
    ASSET_FONT = 5,
    ASSET_LUA_SCRIPT = 6,
    ASSET_CONFIG_JSON = 7,
    ASSET_AUDIO = 8,
    ASSET_ANIMATION = 9,
};

// Compression types
enum class CompressionType : uint32_t {
    NONE = 0,
    DEFLATE = 1,
    LZ4 = 2,
};

// Flags for asset pack features
enum AssetPackFlags : uint32_t {
    FLAG_NONE = 0,
    FLAG_COMPRESSED = 1 << 0,
    FLAG_ENCRYPTED = 1 << 1,
};

// Header structure (must be POD for binary read/write)
#pragma pack(push, 1)
struct AssetPackHeader {
    uint32_t magic;              // "AP67"
    uint32_t version;            // Version 2
    uint32_t asset_count;        // Total number of assets
    uint64_t index_offset;       // Offset to index table
    uint32_t lua_script_count;   // Number of Lua scripts
    uint32_t flags;              // Feature flags
    uint64_t reserved[2];        // Reserved for future use
};

// Index entry for each asset
struct AssetIndexEntry {
    uint64_t path_hash;          // Hash of relative path
    AssetType type;              // Asset type
    uint64_t offset;             // Offset in data section
    uint64_t size;               // Size in bytes (uncompressed)
    uint64_t compressed_size;    // Size in bytes (compressed, 0 if not compressed)
    CompressionType compression; // Compression type
    uint32_t checksum;           // CRC32 checksum
    uint32_t reserved;           // Reserved
};

// Lua script index entry
struct LuaScriptIndexEntry {
    uint64_t path_hash;          // Hash of relative path
    uint64_t offset;             // Offset in data section
    uint64_t size;               // Size in bytes
    uint32_t checksum;           // CRC32 checksum
    uint32_t reserved;           // Reserved
};

// Footer structure
struct AssetPackFooter {
    uint64_t data_checksum;      // Checksum of all asset data
    uint64_t metadata_checksum;  // Checksum of index + lua script index
};
#pragma pack(pop)

// Runtime asset entry (includes path string)
struct AssetEntry {
    std::string path;            // Relative path (e.g., "textures/player.png")
    AssetType type;
    std::vector<uint8_t> data;   // Asset data
    uint64_t path_hash;
    uint32_t checksum;
};

// Runtime Lua script entry
struct LuaScriptEntry {
    std::string path;            // Relative path (e.g., "lua/gameplay/player.lua")
    std::vector<uint8_t> data;   // Script data
    uint64_t path_hash;
    uint32_t checksum;
};

// Helper functions
inline uint64_t HashString(const std::string& str) {
    // Simple FNV-1a hash
    uint64_t hash = 14695981039346656037ULL;
    for (char c : str) {
        hash ^= static_cast<uint64_t>(c);
        hash *= 1099511628211ULL;
    }
    return hash;
}

inline uint32_t CalculateCRC32(const uint8_t* data, size_t length) {
    // Simple CRC32 implementation
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
        }
    }
    return ~crc;
}

inline const char* AssetTypeToString(AssetType type) {
    switch (type) {
        case AssetType::ASSET_TEXTURE: return "Texture";
        case AssetType::ASSET_MODEL: return "Model";
        case AssetType::ASSET_SCENE: return "Scene";
        case AssetType::ASSET_SHADER: return "Shader";
        case AssetType::ASSET_FONT: return "Font";
        case AssetType::ASSET_LUA_SCRIPT: return "Lua Script";
        case AssetType::ASSET_CONFIG_JSON: return "Config JSON";
        case AssetType::ASSET_AUDIO: return "Audio";
        case AssetType::ASSET_ANIMATION: return "Animation";
        default: return "Unknown";
    }
}

} // namespace S67

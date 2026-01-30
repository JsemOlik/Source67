#include "AssetPacker.h"
#include <iostream>
#include <string>
#include <cstring>

void PrintUsage(const char* programName) {
    std::cout << "Source67 Asset Packer Tool\n";
    std::cout << "Usage: " << programName << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -i, --input <dir>         Input assets directory (required)\n";
    std::cout << "  -o, --output <file>       Output asset pack file (required)\n";
    std::cout << "  -c, --compression <type>  Compression type (none, deflate, lz4) [default: none]\n";
    std::cout << "  -v, --verbose             Enable verbose output\n";
    std::cout << "  --validate                Validate the output pack after creation\n";
    std::cout << "  --include-lua             Include Lua scripts (default: yes)\n";
    std::cout << "  --lua-dir <dir>           Lua scripts subdirectory [default: lua]\n";
    std::cout << "  -h, --help                Show this help message\n";
    std::cout << "\nExample:\n";
    std::cout << "  " << programName << " -i assets/ -o GameAssets.apak -c lz4 -v --validate\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 1;
    }

    std::string inputDir;
    std::string outputFile;
    S67::CompressionType compressionType = S67::CompressionType::NONE;
    bool verbose = false;
    bool validate = false;
    bool includeLua = true;
    std::string luaDir = "lua";

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            PrintUsage(argv[0]);
            return 0;
        }
        else if (arg == "-i" || arg == "--input") {
            if (i + 1 < argc) {
                inputDir = argv[++i];
            } else {
                std::cerr << "Error: Missing value for " << arg << "\n";
                return 1;
            }
        }
        else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                outputFile = argv[++i];
            } else {
                std::cerr << "Error: Missing value for " << arg << "\n";
                return 1;
            }
        }
        else if (arg == "-c" || arg == "--compression") {
            if (i + 1 < argc) {
                std::string type = argv[++i];
                if (type == "none") {
                    compressionType = S67::CompressionType::NONE;
                } else if (type == "deflate") {
                    compressionType = S67::CompressionType::DEFLATE;
                } else if (type == "lz4") {
                    compressionType = S67::CompressionType::LZ4;
                } else {
                    std::cerr << "Error: Unknown compression type: " << type << "\n";
                    return 1;
                }
            } else {
                std::cerr << "Error: Missing value for " << arg << "\n";
                return 1;
            }
        }
        else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        }
        else if (arg == "--validate") {
            validate = true;
        }
        else if (arg == "--include-lua") {
            includeLua = true;
        }
        else if (arg == "--lua-dir") {
            if (i + 1 < argc) {
                luaDir = argv[++i];
            } else {
                std::cerr << "Error: Missing value for " << arg << "\n";
                return 1;
            }
        }
        else {
            std::cerr << "Error: Unknown option: " << arg << "\n";
            PrintUsage(argv[0]);
            return 1;
        }
    }

    // Validate required arguments
    if (inputDir.empty()) {
        std::cerr << "Error: Input directory (-i) is required\n";
        PrintUsage(argv[0]);
        return 1;
    }

    if (outputFile.empty()) {
        std::cerr << "Error: Output file (-o) is required\n";
        PrintUsage(argv[0]);
        return 1;
    }

    // Create packer and configure
    S67::AssetPacker packer;
    packer.SetCompressionType(compressionType);
    packer.SetVerbose(verbose);
    packer.SetIncludeLua(includeLua);
    packer.SetLuaDirectory(luaDir);

    // Pack assets
    if (!packer.PackAssets(inputDir, outputFile)) {
        std::cerr << "Error: Failed to pack assets\n";
        return 1;
    }

    // Validate if requested
    if (validate) {
        if (!packer.ValidatePack(outputFile)) {
            std::cerr << "Error: Validation failed\n";
            return 1;
        }
    }

    std::cout << "\nSuccess! Asset pack created: " << outputFile << "\n";
    return 0;
}

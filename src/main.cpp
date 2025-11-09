// RGLite Language Compiler - Main Entry Point
// This file contains the main function and command-line interface

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include "RGLite.h"
#include "ErrorHandler.h"

#ifdef _MSC_VER
// Disable UTF-8/Unicode codepage warning treated as error on some setups
#pragma warning(disable:4819)
#endif

using namespace rglite;

/**
 * @brief Print usage information
 */
void printUsage(const std::string& programName) {
    std::cout << "Usage: " << programName << " [options] <file.rglite>" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help           Show this help message" << std::endl;
    std::cout << "  -v, --version        Show version information" << std::endl;
    std::cout << "  -c, --compile        Compile to bytecode" << std::endl;
    std::cout << "  -o, --output <file>  Specify output file" << std::endl;
    std::cout << "  -d, --debug          Enable debug mode" << std::endl;
    std::cout << "  -s, --strict         Enable strict mode" << std::endl;
    std::cout << "  --repl               Start interactive REPL" << std::endl;
}

/**
 * @brief Print version information
 */
void printVersion() {
    const std::string arch = (sizeof(void*) == 8) ? "x64" : "x86";
    
    std::string os;
#ifdef _WIN32
    os = "Windows";
#elif __linux__
    os = "Linux";
#elif __APPLE__
    os = "macOS";
#else
    os = "Unknown OS";
#endif

    std::cout << "RGLite (TM) RGen Optimizing Compiler Version " << getVersion() << " (" << arch << "-" << os << ")" << std::endl;
    std::cout << "A lightweight RGen language compiler | Built with C++20 standard" << std::endl;
    std::cout << "Copyright (C) 2025 RGLite Developers. All rights reserved." << std::endl;
    std::cout << "\nUsage: rg.exe [ options... ] filename... [ /link link-options... ]" << std::endl;
}

/**
 * @brief Reads the entire content of a text file into a string.
 * 
 * Opens the file in text mode (newline conversion may occur on some platforms).
 * Pre-allocates memory for large files to improve performance.
 * Throws an exception if the file cannot be opened or read (e.g., corruption, permission issues).
 * 
 * @param filename Path to the file to read.
 * @return std::string Entire content of the file as a string.
 * @throws std::runtime_error If file opening or reading fails.
 */
std::string readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::in);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    file.seekg(0, std::ios::end);
    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string content;
    if (size > 0) {
        content.reserve(static_cast<size_t>(size));
    }

    content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

    if (!file) {
        throw std::runtime_error("Failed to read file: " + filename);
    }

    return content;
}

/**
 * @brief Writes bytecode to a file in binary format.
 * 
 * The bytecode is written as raw binary data. Throws an exception if the file
 * cannot be opened or if writing fails (e.g., disk full, permission denied).
 * 
 * @param bytecode The bytecode data to write (vector of uint8_t).
 * @param filename Path to the output file.
 * @throws std::runtime_error If file opening or writing fails.
 */
void writeBytecode(const std::vector<uint8_t>& bytecode, const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot create output file: " + filename);
    }
    
    file.write(reinterpret_cast<const char*>(bytecode.data()), bytecode.size());
    
    if (!file) {
        throw std::runtime_error("Failed to write bytecode to file: " + filename);
    }
}

/**
 * @brief Start interactive REPL
 */
void startRepl() {
    printVersion();
     std::cout << "\nType 'exit' or 'quit' to exit, 'help' for commands.\n";
    
    auto compiler = createCompiler();
    
    while (true) {
        std::cout << ">> ";
        std::string line;
        std::getline(std::cin, line);
        if (!std::getline(std::cin, line)) {
            break;
        }
        if (line.empty()) {
            continue;
        }
        if (line == "exit" || line == "quit") {
            break;
        }
        
        if (line == "help") {
            std::cout << "Commands:\n";
            std::cout << "  exit/quit - Exit the REPL\n";
            std::cout << "  help      - Show help message\n";
            std::cout << "  version   - Show version information\n";
            continue;
        }
        
        if (line == "version") {
            printVersion();
            continue;
        }
        
        try {
            compiler->execute(line);

        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
}

struct Args {
    std::string inputFile;
    std::string outputFile;
    bool compileMode = false;
    bool debugMode = false;
    bool strictMode = false;
    bool replMode = false;
    bool showHelp = false;
    bool showVersion = false;
};

/**
 * @brief parse command line arguments
 * @param argc number of arguments
 * @param argv array of argument strings
 * @return parsed arguments object, or throw exception if parsing fails
 */
Args parseArgs(int argc, char* argv[]) {
    Args args;
    std::vector<std::string> tokens(argv + 1, argv + argc);

    if (tokens.empty()) {
        args.showVersion = true;
        return args;
    }

    bool stopParsing = false;
    for (size_t i = 0; i < tokens.size() && !stopParsing; ++i) {
        const std::string& token = tokens[i];

        if (token == "-h" || token == "--help") {
            if (args.showVersion) {
                throw std::runtime_error("Cannot specify both --help and --version");
            }
            args.showHelp = true;
            stopParsing = true;
        } else if (token == "-v" || token == "--version") {
            if (args.showHelp) {
                throw std::runtime_error("Cannot specify both --help and --version");
            }
            args.showVersion = true;
            stopParsing = true;
        } else if (token == "-c" || token == "--compile") {
            if (args.compileMode) {
                std::cerr << "Warning: --compile specified multiple times; ignoring duplicates\n";
            }
            args.compileMode = true;
        } else if (token == "-o" || token == "--output") {
            if (i + 1 >= tokens.size()) {
                throw std::runtime_error("Missing output filename after " + token);
            }
            if (!args.outputFile.empty()) {
                std::cerr << "Warning: --output specified multiple times; overwriting previous value\n";
            }
            args.outputFile = tokens[++i];
        } else if (token == "-d" || token == "--debug") {
            args.debugMode = true;  // allow multiple times (no side effects)
        } else if (token == "-s" || token == "--strict") {
            args.strictMode = true;  // allow multiple times (no side effects)
        } else if (token == "--repl") {
            if (args.replMode) {
                std::cerr << "Warning: --repl specified multiple times; ignoring duplicates\n";
            }
            args.replMode = true;
        } else if (token[0] != '-') {
            if (!args.inputFile.empty()) {
                throw std::runtime_error("Multiple input files specified (only one allowed)");
            }
            args.inputFile = token;
        } else {
            throw std::runtime_error("Unknown option: " + token);
        }
    }

    return args;
}

/**
 * @brief  Generate default output filename (replace extension with ".rgbc")
 * @param inputPath Input file path
 * @return Default output path (e.g., "test.rgb" → "test.rgbc")
 */
std::string getDefaultOutput(const std::string& inputPath) {
    std::filesystem::path path(inputPath);
    // replace extension with ".rgbc" (no extension adds ".rgbc")
    return (path.parent_path() / (path.stem().string() + ".rgbc")).string();
}

/**
 * @brief Main function
 */
int main(int argc, char* argv[]) {
    try {
        Args args = parseArgs(argc, argv);

        // print help or version if requested (highest priority)
        if (args.showHelp) {
            printUsage(argv[0]);
            return 0;
        }
        if (args.showVersion) {
            printVersion();
            return 0;
        }

        // handle REPL mode (second highest priority)
        if (args.replMode) {
            if (!args.inputFile.empty()) {
                std::cerr << "Warning: REPL mode active; ignoring input file: " << args.inputFile << "\n";
            }
            startRepl();
            return 0;
        }

        // normalize input file to absolute path for Python-like error reporting
        std::string fullInput = args.inputFile;
        try {
            if (!args.inputFile.empty()) {
                fullInput = std::filesystem::absolute(args.inputFile).lexically_normal().string();
            }
        } catch (...) {
            // fall back to original input if normalization fails
        }

        // read source file
        std::string source = readFile(fullInput);

        // configure compiler options
        CompileOptions options;
        options.debug_info = args.debugMode;
        options.strict_mode = args.strictMode;
        auto compiler = createCompiler(options);

        // compile mode: generate bytecode
        if (args.compileMode) {
            std::string outputFile = args.outputFile.empty() 
                ? getDefaultOutput(fullInput) 
                : args.outputFile;

            auto bytecode = compiler->compile(source);
            writeBytecode(bytecode, outputFile);
            std::cout << "Compiled successfully to: " << outputFile << "\n";
        } 
        // execute mode: directly run
        else {
            return compiler->execute(source, fullInput);
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

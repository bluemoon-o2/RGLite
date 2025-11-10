// RGLite Language Compiler - Main Header
// This file defines the core interfaces for the RGLite compiler

#ifndef RGLITE_H
#define RGLITE_H

#include <memory>
#include <string>
#include <vector>

namespace rglite {

// Forward declarations
class Lexer;
class Parser;
class ASTNode;
class SemanticAnalyzer;
class CodeGenerator;
class VM;

/**
 * @brief Compilation options and settings
 */
struct CompileOptions {
    bool optimize = true;           // Enable optimizations
    bool debug_info = false;        // Include debug information
    bool strict_mode = false;       // Enable strict type checking
    int optimization_level = 1;     // Optimization level (0-3)
};

/**
 * @brief Main compiler class that orchestrates the compilation process
 */
class Compiler {
public:
    explicit Compiler(const CompileOptions& options = {});
    ~Compiler();
    
    /**
     * @brief Compile RGLite source code to bytecode
     * @param source The source code to compile
     * @return Bytecode representation
     */
    std::vector<uint8_t> compile(const std::string& source);
    
    /**
     * @brief Execute RGLite source code directly
     * @param source The source code to execute
     * @param filename The filename for error reporting (optional)
     * @return Execution result
     */
    int execute(const std::string& source, const std::string& filename = "<stdin>");
    
    /**
     * @brief Execute precompiled bytecode
     * @param bytecode The bytecode to execute
     * @return Execution result
     */
    int executeBytecode(const std::vector<uint8_t>& bytecode);

    /**
     * @brief Execute precompiled bytecode using a provided VM instance
     * @param bytecode The bytecode to execute
     * @param vm The VM to use for execution (mappings will be applied)
     * @return Execution result
     */
    int executeBytecodeWithVM(const std::vector<uint8_t>& bytecode, VM& vm);
    
private:
    // Helper function to get source line by line number
    std::string getSourceLine(const std::string& source, int line);
    
    // Compilation options applied to this compiler instance
    CompileOptions options_;
};

/**
 * @brief Create a new compiler instance with specified options
 * @param options Compilation options
 * @return Shared pointer to compiler instance
 */
std::shared_ptr<Compiler> createCompiler(const CompileOptions& options = {});

/**
 * @brief Get RGLite version information
 * @return Version string
 */
std::string getVersion();

/**
 * @brief Check if RGLite is built with specific feature
 * @param feature Feature name to check
 * @return True if feature is available
 */
bool hasFeature(const std::string& feature);

} // namespace rglite

#endif // RGLITE_H

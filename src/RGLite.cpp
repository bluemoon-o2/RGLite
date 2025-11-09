// RGLite Language Compiler - Core Implementation
// This file implements the core interfaces for the RGLite compiler

#include "RGLite.h"
#include "Lexer.h"
#include "Parser.h"
#include "SemanticAnalyzer.h"
#include "CodeGenerator.h"
#include "Bytecode.h"
#include "VM.h"
#include "ErrorHandler.h"
#include <filesystem>
#include <memory>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>

namespace rglite {

// Forward declarations for compiler components
class Lexer;
class Parser;
class SemanticAnalyzer;
class CodeGenerator;
class VM;

// Compiler implementation
Compiler::Compiler(const CompileOptions& options)
    : options_(options) {
    // Components are instantiated per compilation for isolation and fresh diagnostics.
}

Compiler::~Compiler() = default;

std::vector<uint8_t> Compiler::compile(const std::string& source) {
    // Compile the source into bytecode vector by serializing Chunk
    // 1) Lex -> Parse -> Semantics -> Codegen
    try {
        // Normalize filename to absolute path for better diagnostics
        std::string fullFilename = "<stdin>";
        // Create lexer with filename for error reporting
        auto lexer = std::make_unique<Lexer>(source, fullFilename);

        // Create error handler
        auto errorHandler = std::make_shared<StandardErrorHandler>();

        // Create parser with error handler
        auto parser = std::make_unique<Parser>(std::move(lexer), errorHandler);

        // Parse the source code into an AST
        auto ast = parser->parse();
        if (errorHandler->hasErrors()) {
            const auto& diagnostics = errorHandler->getDiagnostics();
            const Diagnostic* selected = nullptr;
            for (const auto& d : diagnostics) {
                if (d.severity == Severity::ERROR && d.column > 0) {
                    selected = &d; // keep the last matching error
                }
            }
            if (!selected) {
                for (const auto& d : diagnostics) {
                    if (d.severity == Severity::ERROR) { selected = &d; break; }
                }
            }
            if (selected) {
                std::cerr << selected->toString() << std::endl;
            }
            return {};
        }

        // Run semantic analysis
        auto semantic = std::make_unique<SemanticAnalyzer>(errorHandler);
        semantic->setSource(source, fullFilename);
        (void)semantic->analyze(ast);
        if (errorHandler->hasErrors()) {
            const auto& diagnostics = errorHandler->getDiagnostics();
            const Diagnostic* selected = nullptr;
            for (const auto& d : diagnostics) {
                if (d.severity == Severity::ERROR && d.column > 0) { selected = &d; }
            }
            if (!selected) {
                for (const auto& d : diagnostics) {
                    if (d.severity == Severity::ERROR) { selected = &d; break; }
                }
            }
            if (selected) {
                std::cerr << selected->toString() << std::endl;
            }
            return {};
        }

        // Convert unique_ptr to shared_ptr for codegen API
        std::shared_ptr<ASTNode> astNode = std::move(ast);

        // Create code generator and generate bytecode
        auto codegen = createCodeGenerator();
        (void)codegen->generate(astNode);
        if (codegen->hasErrors()) {
            std::cerr << "Code generation errors occurred" << std::endl;
            return {};
        }

        // Serialize chunk to a byte vector
        auto chunkPtr = codegen->getBytecode();

        if (options_.debug_info) {
            const auto& constsDbg = chunkPtr->getConstants();
            const auto& instrsDbg = chunkPtr->getInstructions();
            std::cout << "[debug] constants=" << constsDbg.size()
                      << ", instructions=" << instrsDbg.size() << std::endl;
        }

        // --- Serialization helpers ---
        auto writeUint8 = [](std::vector<uint8_t>& out, uint8_t v) {
            out.push_back(v);
        };
        auto writeUint32 = [](std::vector<uint8_t>& out, uint32_t v) {
            for (int i = 0; i < 4; ++i) out.push_back(static_cast<uint8_t>((v >> (i * 8)) & 0xFF));
        };
        auto writeUint64 = [](std::vector<uint8_t>& out, uint64_t v) {
            for (int i = 0; i < 8; ++i) out.push_back(static_cast<uint8_t>((v >> (i * 8)) & 0xFF));
        };
        auto writeDouble = [&](std::vector<uint8_t>& out, double d) {
            static_assert(sizeof(double) == 8, "double must be 8 bytes");
            uint64_t u = 0;
            std::memcpy(&u, &d, sizeof(double));
            writeUint64(out, u);
        };
        auto writeString = [&](std::vector<uint8_t>& out, const std::string& s) {
            writeUint32(out, static_cast<uint32_t>(s.size()));
            out.insert(out.end(), s.begin(), s.end());
        };

        auto serializeValue = [&](std::vector<uint8_t>& out, const Value& v) {
            writeUint8(out, static_cast<uint8_t>(v.getType()));
            switch (v.getType()) {
                case ValueType::NIL:
                    break;
                case ValueType::BOOLEAN:
                    writeUint8(out, v.asBoolean() ? 1 : 0);
                    break;
                case ValueType::INTEGER:
                    writeUint64(out, static_cast<uint64_t>(v.asInteger()));
                    break;
                case ValueType::FLOAT:
                    writeDouble(out, v.asFloat());
                    break;
                case ValueType::STRING:
                    writeString(out, v.asString());
                    break;
                case ValueType::LIST:
                case ValueType::DICT:
                case ValueType::TUPLE:
                case ValueType::SET:
                case ValueType::FUNCTION:
                case ValueType::ITERATOR:
                    writeUint32(out, v.asIndex());
                    break;
                case ValueType::NATIVE_FUNCTION:
                    writeString(out, v.asNativeFunctionName());
                    break;
                case ValueType::EXCEPTION:
                    // Store message if available; else store empty
                    writeString(out, v.asString());
                    break;
                default:
                    break;
            }
        };

        std::vector<uint8_t> out;
        // Header: magic + version
        writeString(out, std::string("RGLB"));
        writeUint32(out, 1); // version
        // Counts
        const auto& consts = chunkPtr->getConstants();
        const auto& instrs = chunkPtr->getInstructions();
        writeUint32(out, static_cast<uint32_t>(consts.size()));
        writeUint32(out, static_cast<uint32_t>(instrs.size()));
        // Constants
        for (const auto& c : consts) {
            serializeValue(out, c);
        }
        // Instructions: opcode (u8), operand (u32), line (u32)
        for (const auto& ins : instrs) {
            writeUint8(out, static_cast<uint8_t>(ins.opcode));
            writeUint32(out, ins.operand);
            writeUint32(out, ins.line);
        }

        return out;
    } catch (const std::exception& e) {
        std::cerr << "Error during compilation: " << e.what() << std::endl;
        return {};
    }
}

std::string Compiler::getSourceLine(const std::string& source, int line) {
    std::istringstream iss(source);
    std::string currentLine;
    int currentLineNum = 1;
    
    while (std::getline(iss, currentLine)) {
        if (currentLineNum == line) {
            return currentLine;
        }
        currentLineNum++;
    }
    
    return "";  // Return empty string if line not found
}

int Compiler::execute(const std::string& source, const std::string& filename) {
    try {
        // Normalize filename to absolute path for Python-like error messages
        std::string fullFilename = filename;
        try {
            if (!filename.empty()) {
                fullFilename = std::filesystem::absolute(filename).lexically_normal().string();
            }
        } catch (...) {
            // If normalization fails, fall back to original filename
        }
        // Create lexer with filename for error reporting
        auto lexer = std::make_unique<Lexer>(source, fullFilename);
        
        // Create error handler
        auto errorHandler = std::make_shared<StandardErrorHandler>();
        
        // Create parser with error handler
        auto parser = std::make_unique<Parser>(std::move(lexer), errorHandler);
        
        // Parse the source code into an AST
        auto ast = parser->parse();
        if (errorHandler->hasErrors()) {
            const auto& diagnostics = errorHandler->getDiagnostics();
            
            // Prefer the most informative syntax error: choose the last ERROR with a non-zero column.
            // This helps align the caret with the actual offending token (Python-like behavior).
            const Diagnostic* selected = nullptr;
            for (const auto& d : diagnostics) {
                if (d.severity == Severity::ERROR && d.column > 0) {
                    selected = &d; // keep the last matching error
                }
            }
            // Fallback to the first ERROR if none has a column
            if (!selected) {
                for (const auto& d : diagnostics) {
                    if (d.severity == Severity::ERROR) {
                        selected = &d;
                        break;
                    }
                }
            }
            
            if (selected) {
                // Use the Diagnostic's toString method for consistent formatting
                // The toString() method already includes the traceback header
                std::cerr << selected->toString() << std::endl;
            }
            return 1;
        }

        // Run semantic analysis to catch errors like 'return' outside function
        auto semantic = std::make_unique<SemanticAnalyzer>(errorHandler);
        // Provide source and filename so diagnostics can include caret and source line
        // Note: SemanticAnalyzer will use this to populate Diagnostic::sourceLine/column
        semantic->setSource(source, fullFilename);
        (void)semantic->analyze(ast);
        if (errorHandler->hasErrors()) {
            const auto& diagnostics = errorHandler->getDiagnostics();

            // Prefer the most informative error: choose the last ERROR with a non-zero column.
            const Diagnostic* selected = nullptr;
            for (const auto& d : diagnostics) {
                if (d.severity == Severity::ERROR && d.column > 0) {
                    selected = &d; // keep the last matching error
                }
            }
            // Fallback to the first ERROR if none has a column
            if (!selected) {
                for (const auto& d : diagnostics) {
                    if (d.severity == Severity::ERROR) {
                        selected = &d;
                        break;
                    }
                }
            }

            if (selected) {
                std::cerr << selected->toString() << std::endl;
            }
            return 1;
        }

        // Convert unique_ptr to shared_ptr
        std::shared_ptr<ASTNode> astNode = std::move(ast);
        
        // Create code generator
        auto codegen = createCodeGenerator();

        // Create VM and attach to code generator BEFORE code generation, so variable names map correctly
        VM vm(codegen.get());
        codegen->setVM(&vm);

        // Generate bytecode from AST
        auto function = codegen->generate(astNode);
        if (codegen->hasErrors()) {
            std::cerr << "Code generation errors occurred" << std::endl;
            return 1;
        }
        
        // Get the bytecode chunk
        auto chunk = codegen->getBytecode();
        
        if (options_.debug_info) {
            const auto& constsDbg = chunk->getConstants();
            const auto& instrsDbg = chunk->getInstructions();
            std::cout << "[debug] file=" << fullFilename
                      << ", constants=" << constsDbg.size()
                      << ", instructions=" << instrsDbg.size() << std::endl;
        }

        // Execute the bytecode
        bool success = vm.interpret(*chunk, fullFilename);
        if (!success) {
            return 1;
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error during execution: " << e.what() << std::endl;
        return 1;
    }
}

int Compiler::executeBytecode(const std::vector<uint8_t>& bytecode) {
    // Deserialize bytecode vector into a Chunk, then interpret with VM
    try {
        auto readUint8 = [](const std::vector<uint8_t>& in, size_t& off, uint8_t& out) -> bool {
            if (off + 1 > in.size()) return false; out = in[off]; off += 1; return true;
        };
        auto readUint32 = [](const std::vector<uint8_t>& in, size_t& off, uint32_t& out) -> bool {
            if (off + 4 > in.size()) return false; out = 0; for (int i=0;i<4;++i) out |= static_cast<uint32_t>(in[off+i]) << (i*8); off += 4; return true;
        };
        auto readUint64 = [](const std::vector<uint8_t>& in, size_t& off, uint64_t& out) -> bool {
            if (off + 8 > in.size()) return false; out = 0; for (int i=0;i<8;++i) out |= static_cast<uint64_t>(in[off+i]) << (i*8); off += 8; return true;
        };
        auto readDouble = [&](const std::vector<uint8_t>& in, size_t& off, double& out) -> bool {
            uint64_t u = 0; if (!readUint64(in, off, u)) return false; std::memcpy(&out, &u, sizeof(double)); return true;
        };
        auto readString = [&](const std::vector<uint8_t>& in, size_t& off, std::string& out) -> bool {
            uint32_t len = 0; if (!readUint32(in, off, len)) return false; if (off + len > in.size()) return false; out.assign(reinterpret_cast<const char*>(&in[off]), len); off += len; return true;
        };

        auto deserializeValue = [&](const std::vector<uint8_t>& in, size_t& off, Value& v) -> bool {
            uint8_t t = 0; if (!readUint8(in, off, t)) return false; auto type = static_cast<ValueType>(t);
            switch (type) {
                case ValueType::NIL:
                    v = Value(); return true;
                case ValueType::BOOLEAN: {
                    uint8_t b=0; if (!readUint8(in, off, b)) return false; v = Value(b != 0); return true; }
                case ValueType::INTEGER: {
                    uint64_t u=0; if (!readUint64(in, off, u)) return false; v = Value(static_cast<int64_t>(u)); return true; }
                case ValueType::FLOAT: {
                    double d=0.0; if (!readDouble(in, off, d)) return false; v = Value(d); return true; }
                case ValueType::STRING: {
                    std::string s; if (!readString(in, off, s)) return false; v = Value(s); return true; }
                case ValueType::LIST:
                case ValueType::DICT:
                case ValueType::TUPLE:
                case ValueType::SET:
                case ValueType::FUNCTION:
                case ValueType::ITERATOR: {
                    uint32_t idx=0; if (!readUint32(in, off, idx)) return false; v = Value(idx, type); return true; }
                case ValueType::NATIVE_FUNCTION: {
                    std::string name; if (!readString(in, off, name)) return false; v = Value(name, true); return true; }
                case ValueType::EXCEPTION: {
                    std::string msg; if (!readString(in, off, msg)) return false; v = Value(msg.c_str()); return true; }
                default:
                    return false;
            }
        };

        size_t off = 0;
        std::string magic;
        if (!readString(bytecode, off, magic)) {
            std::cerr << "Invalid bytecode: missing header" << std::endl;
            return 1;
        }
        if (magic != "RGLB") {
            std::cerr << "Invalid bytecode: bad magic" << std::endl;
            return 1;
        }
        uint32_t version = 0; if (!readUint32(bytecode, off, version)) { std::cerr << "Invalid bytecode: missing version" << std::endl; return 1; }
        if (version != 1) {
            std::cerr << "Unsupported bytecode version: " << version << std::endl;
            return 1;
        }
        uint32_t constCount = 0, instrCount = 0;
        if (!readUint32(bytecode, off, constCount) || !readUint32(bytecode, off, instrCount)) {
            std::cerr << "Invalid bytecode: missing counts" << std::endl;
            return 1;
        }

        Chunk chunk;
        // Constants
        for (uint32_t i = 0; i < constCount; ++i) {
            Value v; if (!deserializeValue(bytecode, off, v)) { std::cerr << "Invalid bytecode: constant parse error" << std::endl; return 1; }
            (void)chunk.addConstant(v);
        }
        // Instructions
        for (uint32_t i = 0; i < instrCount; ++i) {
            uint8_t opc=0; uint32_t operand=0; uint32_t line=1;
            if (!readUint8(bytecode, off, opc) || !readUint32(bytecode, off, operand) || !readUint32(bytecode, off, line)) {
                std::cerr << "Invalid bytecode: instruction parse error" << std::endl;
                return 1;
            }
            Instruction ins(static_cast<OpCode>(opc), operand, line);
            (void)chunk.addInstruction(ins);
        }

        VM vm; // Builtins are registered in VM constructor
        bool success = vm.interpret(chunk, "<bytecode>");
        return success ? 0 : 1;
    } catch (const std::exception& e) {
        std::cerr << "Error during bytecode execution: " << e.what() << std::endl;
        return 1;
    }
}

// Factory function implementation
std::shared_ptr<Compiler> createCompiler(const CompileOptions& options) {
    auto compiler = std::make_shared<Compiler>(options);
    return compiler;
}

// Version information
std::string getVersion() {
    return "alpha1";
}

// Feature detection
bool hasFeature(const std::string& feature) {
    // Minimal feature detection
    if (feature == "bytecode" || feature == "repl" || feature == "compile" || feature == "execute_bytecode") {
        return true;
    }
    return false;
}

} // namespace rglite

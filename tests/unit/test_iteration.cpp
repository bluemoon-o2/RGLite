// test_iteration.cpp - Tests for iter, next, enumerate, zip, map, filter (parse .rgb)

#include "TestFramework.h"
#include "VM.h"
#include "Lexer.h"
#include "Parser.h"
#include "SemanticAnalyzer.h"
#include "CodeGenerator.h"
#include "ErrorHandler.h"

#include <filesystem>
#include <fstream>
#include <sstream>

using namespace rglite;

static std::string resourcePath(const std::string& name) {
    std::filesystem::path p(__FILE__);
    return (p.parent_path() / "iteration_sources" / name).string();
}

static std::unique_ptr<VM> runRgbFile(const std::string& filepath) {
    std::ifstream ifs(filepath);
    if (!ifs) {
        return nullptr;
    }
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    std::string source = buffer.str();

    auto errorHandler = std::make_shared<StandardErrorHandler>();
    auto lexer = std::make_unique<Lexer>(source, filepath, errorHandler);
    auto parser = std::make_unique<Parser>(std::move(lexer), errorHandler);
    auto ast = parser->parse();
    if (errorHandler->hasErrors()) {
        return nullptr;
    }

    auto semantic = std::make_unique<SemanticAnalyzer>(errorHandler);
    semantic->setSource(source, filepath);
    (void)semantic->analyze(ast);
    if (errorHandler->hasErrors()) {
        return nullptr;
    }

    std::shared_ptr<ASTNode> astNode = std::move(ast);
    auto codegen = createCodeGenerator();

    auto vm = std::make_unique<VM>();
    codegen->setVM(vm.get());
    (void)codegen->generate(astNode);
    if (codegen->hasErrors()) {
        return nullptr;
    }

    auto chunk = codegen->getBytecode();
    if (!chunk) {
        return nullptr;
    }

    // Apply variable name mapping for globals
    for (const auto& p : codegen->getVariableNameTable()) {
        vm->setVariableName(p.first, p.second);
    }

    bool ok = vm->interpret(*chunk, filepath);
    if (!ok) {
        return nullptr;
    }
    return vm;
}

TEST(Iteration, EnumerateList) {
    auto vm = runRgbFile(resourcePath("enumerate_list.rgb"));
    EXPECT_TRUE(vm != nullptr);
    if (!vm) return;
    Value result = vm->getGlobal("result");
    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(1, result.asInteger());
}

TEST(Iteration, ZipListsLength) {
    auto vm = runRgbFile(resourcePath("zip_lists_length.rgb"));
    EXPECT_TRUE(vm != nullptr);
    if (!vm) return;
    Value result = vm->getGlobal("result");
    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(2, result.asInteger());
}

TEST(Iteration, MapStr) {
    auto vm = runRgbFile(resourcePath("map_str.rgb"));
    EXPECT_TRUE(vm != nullptr);
    if (!vm) return;
    Value result = vm->getGlobal("result");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(std::string("2"), result.asString());
}

TEST(Iteration, FilterIsNumber) {
    auto vm = runRgbFile(resourcePath("filter_isnumber.rgb"));
    EXPECT_TRUE(vm != nullptr);
    if (!vm) return;
    Value lenVal = vm->getGlobal("len_result");
    Value second = vm->getGlobal("second");
    EXPECT_TRUE(lenVal.isInteger());
    EXPECT_EQ(2, lenVal.asInteger());
    EXPECT_TRUE(second.isFloat());
    EXPECT_EQ(2.5, second.asFloat());
}

TEST(Iteration, NextOnStringWithDefault) {
    auto vm = runRgbFile(resourcePath("next_string_default.rgb"));
    EXPECT_TRUE(vm != nullptr);
    if (!vm) return;
    Value val = vm->getGlobal("val");
    EXPECT_TRUE(val.isString());
    EXPECT_EQ(std::string("X"), val.asString());
}

// Main test runner
RUN_ALL_TESTS()

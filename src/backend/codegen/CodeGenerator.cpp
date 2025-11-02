// CodeGenerator.cpp - Bytecode code generator implementation for RGLite
// This file implements the code generator that converts AST to bytecode

#include "CodeGenerator.h"
#include <sstream>
#include <limits>

namespace rglite {

// CodeGenerator implementation
CodeGenerator::CodeGenerator() 
    : currentChunk_(nullptr), nextVariableIndex_(0), scopeDepth_(0) {
}

std::shared_ptr<Function> CodeGenerator::generate(const std::shared_ptr<ASTNode>& ast) {
    // Create a new function for the program
    currentFunction_ = std::make_shared<Function>("__main__", 0);
    currentChunk_ = &currentFunction_->getChunk();
    
    // Clear any previous state
    variables_.clear();
    nextVariableIndex_ = 0;
    scopeDepth_ = 0;
    errors_.clear();
    
    // Generate bytecode for the AST
    if (ast) {
        ast->accept(*this);
    }
    
    // Add a HALT instruction at the end
    emitInstruction(OpCode::HALT);
    
    return currentFunction_;
}

std::shared_ptr<Chunk> CodeGenerator::getBytecode() const {
    if (currentFunction_) {
        // Return a copy of the chunk
        return std::make_shared<Chunk>(currentFunction_->getChunk());
    }
    return nullptr;
}

// Expression visitors
void CodeGenerator::visitLiteralExpr(LiteralExpr& expr) {
    // Convert token to Value based on token type
    Value value;
    
    switch (expr.token.type) {
        case TokenType::INTEGER:
            value = Value(static_cast<int64_t>(expr.token.int_value));
            break;
        case TokenType::FLOAT:
            value = Value(expr.token.float_value);
            break;
        case TokenType::STRING:
            value = Value(expr.token.lexeme);
            break;
        case TokenType::KW_TRUE:
            value = Value(true);
            break;
        case TokenType::KW_FALSE:
            value = Value(false);
            break;
        case TokenType::KW_NONE:
            value = Value();  // Use default constructor for NIL value
            break;
        default:
            // For other token types, use the lexeme as a string
            value = Value(expr.token.lexeme);
            break;
    }
    
    // Emit a LOAD_CONST instruction with the value
    size_t constIndex = emitConstant(value);
    // Check if constIndex fits in uint32_t to avoid truncation
    if (constIndex > std::numeric_limits<uint32_t>::max()) {
        error("Constant index too large", expr.token.location);
        return;
    }
    emitInstruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constIndex));
}

void CodeGenerator::visitIdentifierExpr(IdentifierExpr& expr) {
    // Emit a LOAD_VAR instruction with the variable index
    size_t varIndex = getVariableIndex(expr.name);
    // Check if varIndex fits in uint32_t to avoid truncation
    if (varIndex > std::numeric_limits<uint32_t>::max()) {
        error("Variable index too large", expr.location);
        return;
    }
    emitInstruction(OpCode::LOAD_VAR, static_cast<uint32_t>(varIndex));
}

void CodeGenerator::visitBinaryExpr(BinaryExpr& expr) {
    // Generate bytecode for the right operand first (to maintain order)
    expr.right->accept(*this);
    
    // Generate bytecode for the left operand
    expr.left->accept(*this);
    
    // Emit the appropriate binary operation instruction
    switch (expr.op.type) {
        case TokenType::OP_PLUS:
            emitInstruction(OpCode::ADD);
            break;
        case TokenType::OP_MINUS:
            emitInstruction(OpCode::SUB);
            break;
        case TokenType::OP_MULTIPLY:
            emitInstruction(OpCode::MUL);
            break;
        case TokenType::OP_DIVIDE:
            emitInstruction(OpCode::DIV);
            break;
        case TokenType::OP_MODULO:
            emitInstruction(OpCode::MOD);
            break;
        case TokenType::OP_EQUAL:
            emitInstruction(OpCode::EQ);
            break;
        case TokenType::OP_NOT_EQUAL:
            emitInstruction(OpCode::NEQ);
            break;
        case TokenType::OP_LESS:
            emitInstruction(OpCode::LT);
            break;
        case TokenType::OP_LESS_EQUAL:
            emitInstruction(OpCode::LTE);
            break;
        case TokenType::OP_GREATER:
            emitInstruction(OpCode::GT);
            break;
        case TokenType::OP_GREATER_EQUAL:
            emitInstruction(OpCode::GTE);
            break;
        case TokenType::KW_AND:
            emitInstruction(OpCode::AND);
            break;
        case TokenType::KW_OR:
            emitInstruction(OpCode::OR);
            break;
        default:
            error("Unknown binary operator: " + expr.op.lexeme, expr.op.location);
            break;
    }
}

void CodeGenerator::visitCallExpr(CallExpr& expr) {
    // Generate bytecode for the arguments
    for (const auto& argument : expr.arguments) {
        argument->accept(*this);
    }
    
    // Generate bytecode for the callee
    expr.callee->accept(*this);
    
    // Emit a CALL instruction with the argument count
    // Check if arguments size fits in uint32_t to avoid truncation
    if (expr.arguments.size() > std::numeric_limits<uint32_t>::max()) {
        error("Too many arguments", expr.location);
        return;
    }
    emitInstruction(OpCode::CALL, static_cast<uint32_t>(expr.arguments.size()));
}

void CodeGenerator::visitUnaryExpr(UnaryExpr& expr) {
    // Generate bytecode for the operand
    expr.operand->accept(*this);
    
    // Emit the appropriate unary operation instruction
    if (expr.op == "-") {
        emitInstruction(OpCode::SUB);
    } else if (expr.op == "not") {
        emitInstruction(OpCode::NOT);
    } else {
        error("Unknown unary operator: " + expr.op, expr.location);
    }
}

// Statement visitors
void CodeGenerator::visitExprStmt(ExprStmt& stmt) {
    // Generate bytecode for the expression
    stmt.expression->accept(*this);
    
    // Pop the result if it's not used
    emitInstruction(OpCode::POP);
}

void CodeGenerator::visitBlockStmt(BlockStmt& stmt) {
    // Begin a new scope
    beginScope();
    
    // Generate bytecode for each statement in the block
    for (const auto& statement : stmt.statements) {
        statement->accept(*this);
    }
    
    // End the scope
    endScope();
}

void CodeGenerator::visitIfStmt(IfStmt& stmt) {
    // Generate bytecode for the condition
    stmt.condition->accept(*this);
    
    // Emit a JUMP_IF_FALSE instruction with a placeholder address
    size_t elseJump = currentChunk_->addInstruction(Instruction(OpCode::JUMP_IF_FALSE, 0));
    
    // Emit a POP instruction to discard the condition value
    emitInstruction(OpCode::POP);
    
    // Generate bytecode for the then branch
    stmt.thenBranch->accept(*this);
    
    // Emit a JUMP instruction to skip the else branch
    size_t endJump = currentChunk_->addInstruction(Instruction(OpCode::JUMP, 0));
    
    // Patch the else jump to jump to the else branch or end
    patchJump(elseJump);
    
    // Emit a POP instruction to discard the condition value
    emitInstruction(OpCode::POP);
    
    // Generate bytecode for the else branch if present
    if (stmt.elseBranch) {
        stmt.elseBranch->accept(*this);
    }
    
    // Patch the end jump to jump to the end
    patchJump(endJump);
}

void CodeGenerator::visitWhileStmt(WhileStmt& stmt) {
    // Remember the start of the loop
    size_t loopStart = currentChunk_->size();
    
    // Generate bytecode for the condition
    stmt.condition->accept(*this);
    
    // Emit a JUMP_IF_FALSE instruction to exit the loop
    size_t exitJump = currentChunk_->addInstruction(Instruction(OpCode::JUMP_IF_FALSE, 0));
    
    // Emit a POP instruction to discard the condition value
    emitInstruction(OpCode::POP);
    
    // Generate bytecode for the loop body
    stmt.body->accept(*this);
    
    // Emit a JUMP instruction to go back to the start of the loop
    // Check if loopStart fits in uint32_t to avoid truncation
    if (loopStart > std::numeric_limits<uint32_t>::max()) {
        error("Loop start position too large", stmt.location);
        return;
    }
    emitInstruction(OpCode::JUMP, static_cast<uint32_t>(loopStart));
    
    // Patch the exit jump to jump to the end of the loop
    patchJump(exitJump);
    
    // Emit a POP instruction to discard the condition value
    emitInstruction(OpCode::POP);
}

void CodeGenerator::visitFunctionDeclStmt(FunctionDeclStmt& stmt) {
    // Create a new function
    // Check if parameters size fits in int to avoid truncation
    if (stmt.parameters.size() > std::numeric_limits<int>::max()) {
        error("Too many parameters", stmt.location);
        return;
    }
    auto function = std::make_shared<Function>(stmt.name, static_cast<int>(stmt.parameters.size()));
    
    // Save the current function and chunk
    auto previousFunction = currentFunction_;
    auto previousChunk = currentChunk_;
    
    // Set the new function as current
    currentFunction_ = function;
    currentChunk_ = &function->getChunk();
    
    // Begin a new scope for the function
    beginScope();
    
    // Add parameters to the scope
    for (const auto& parameter : stmt.parameters) {
        createVariable(parameter);
    }
    
    // Generate bytecode for the function body
    stmt.body->accept(*this);
    
    // Emit a RETURN instruction if the function doesn't end with one
    if (stmt.body->statements.empty() || 
        (stmt.body->statements.back()->getType() != StmtType::RETURN_STMT)) {
        emitInstruction(OpCode::RETURN);
    }
    
    // End the scope
    endScope();
    
    // Restore the previous function and chunk
    currentFunction_ = previousFunction;
    currentChunk_ = previousChunk;
    
    // Add the function to the constant pool
    size_t functionIndex = emitConstant(Value(static_cast<uint32_t>(0), ValueType::FUNCTION));
    
    // Create a variable for the function name
    size_t varIndex = createVariable(stmt.name);
    
    // Emit a LOAD_CONST instruction with the function
    // Check if functionIndex fits in uint32_t to avoid truncation
    if (functionIndex > std::numeric_limits<uint32_t>::max()) {
        error("Function index too large", stmt.location);
        return;
    }
    emitInstruction(OpCode::LOAD_CONST, static_cast<uint32_t>(functionIndex));
    
    // Emit a STORE_VAR instruction to store the function in the variable
    // Check if varIndex fits in uint32_t to avoid truncation
    if (varIndex > std::numeric_limits<uint32_t>::max()) {
        error("Variable index too large", stmt.location);
        return;
    }
    emitInstruction(OpCode::STORE_VAR, static_cast<uint32_t>(varIndex));
}

void CodeGenerator::visitReturnStmt(ReturnStmt& stmt) {
    // Generate bytecode for the return value if present
    if (stmt.value) {
        stmt.value->accept(*this);
    } else {
        // Load a nil value if no return value
        // Use the default constructor which creates a NIL value
        Value nilValue;
        size_t nilIndex = emitConstant(nilValue);
        // Check if nilIndex fits in uint32_t to avoid truncation
        if (nilIndex > std::numeric_limits<uint32_t>::max()) {
            error("Nil constant index too large", stmt.location);
            return;
        }
        emitInstruction(OpCode::LOAD_CONST, static_cast<uint32_t>(nilIndex));
    }
    
    // Emit a RETURN instruction
    emitInstruction(OpCode::RETURN);
}

// Private helper methods
void CodeGenerator::emitInstruction(OpCode opcode, uint32_t operand) {
    if (currentChunk_) {
        currentChunk_->addInstruction(Instruction(opcode, operand));
    }
}

size_t CodeGenerator::emitConstant(const Value& value) {
    if (currentChunk_) {
        return currentChunk_->addConstant(value);
    }
    return 0;
}

void CodeGenerator::error(const std::string& message, const SourceLocation& pos) {
    std::ostringstream oss;
    oss << "Code generation error";
    if (pos.line > 0) {
        oss << " at line " << pos.line;
        if (pos.column > 0) {
            oss << ", column " << pos.column;
        }
    }
    oss << ": " << message;
    errors_.push_back(oss.str());
}

size_t CodeGenerator::getVariableIndex(const std::string& name) {
    auto it = variables_.find(name);
    if (it != variables_.end()) {
        return it->second.index;
    }
    
    // Create a new variable if it doesn't exist
    return createVariable(name);
}

size_t CodeGenerator::createVariable(const std::string& name) {
    VariableInfo info;
    info.index = nextVariableIndex_++;
    info.isCaptured = false;
    variables_[name] = info;
    return info.index;
}


void CodeGenerator::patchJump(size_t instructionIndex) {
    if (currentChunk_) {
        // Get the instruction to patch
        Instruction& instr = const_cast<Instruction&>(currentChunk_->getInstruction(instructionIndex));
        
        // Set the jump offset to the current position
        size_t offset = currentChunk_->size() - instructionIndex - 1;
        // Check if offset fits in uint32_t to avoid truncation
        if (offset > std::numeric_limits<uint32_t>::max()) {
            error("Jump offset too large");
            return;
        }
        instr.operand = static_cast<uint32_t>(offset);
    }
}

void CodeGenerator::beginScope() {
    scopeDepth_++;
}

void CodeGenerator::endScope() {
    scopeDepth_--;
    
    // In a full implementation, we would remove variables from the symbol table
    // that are no longer in scope
}

// Factory function
std::shared_ptr<CodeGenerator> createCodeGenerator() {
    return std::make_shared<CodeGenerator>();
}

} // namespace rglite
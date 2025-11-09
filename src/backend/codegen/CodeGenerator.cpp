// CodeGenerator.cpp - Bytecode code generator implementation for RGLite
// This file implements the code generator that converts AST to bytecode

#include "CodeGenerator.h"
#include "VM.h"
#include <sstream>
#include <limits>
#include <unordered_set>

namespace rglite {

// CodeGenerator implementation
CodeGenerator::CodeGenerator() 
    : currentFunction_(nullptr), currentChunk_(nullptr), nextVariableIndex_(0), scopeDepth_(0), vm_(nullptr) {
}

// Set VM instance for variable name mapping
void CodeGenerator::setVM(VM* vm) {
    vm_ = vm;
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

std::shared_ptr<Function> CodeGenerator::getFunctionByIndex(size_t index) const {
    if (index < functions_.size()) {
        return functions_[index];
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
            // For string literals, the lexeme should contain the actual string content
            // (without quotes) as set by the Lexer's makeToken method
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
    // Check if this is a builtin function
    if (isBuiltinFunction(expr.name)) {
        // For builtin functions, create a Value with the function name and mark it as native
        size_t constIndex = emitConstant(Value(expr.name, true));
        // Check if constIndex fits in uint32_t to avoid truncation
        if (constIndex > std::numeric_limits<uint32_t>::max()) {
            error("Constant index too large", expr.location);
            return;
        }
        emitInstruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constIndex));
    } else {
        // For regular variables, emit a LOAD_VAR instruction with the variable index
        size_t varIndex = getVariableIndex(expr.name);
        // Check if varIndex fits in uint32_t to avoid truncation
        if (varIndex > std::numeric_limits<uint32_t>::max()) {
            error("Variable index too large", expr.location);
            return;
        }
        emitInstruction(OpCode::LOAD_VAR, static_cast<uint32_t>(varIndex));
    }
}

void CodeGenerator::visitBinaryExpr(BinaryExpr& expr) {
    
    
    // Check if this is an assignment operation
    if (expr.op.type == TokenType::OP_ASSIGN) {
        // For assignment, evaluate the right side first
        expr.right->accept(*this);
        
        // Check if left side is an identifier
        if (auto identifier = dynamic_cast<IdentifierExpr*>(expr.left.get())) {
            // Get or create the variable
            size_t varIndex = getVariableIndex(identifier->name);
            
            // Check if varIndex fits in uint32_t to avoid truncation
            if (varIndex > std::numeric_limits<uint32_t>::max()) {
                error("Variable index too large", expr.op.location);
                return;
            }
            
            // Emit a STORE_VAR instruction to store the value in the variable
            emitInstruction(OpCode::STORE_VAR, static_cast<uint32_t>(varIndex));
            
            // Leave the value on the stack (Python-style assignment returns the assigned value)
            return;
        } else {
            error("Invalid assignment target", expr.op.location);
            return;
        }
    }
    
    // For non-assignment binary operations, generate bytecode for the left operand first
    expr.left->accept(*this);
    
    // Generate bytecode for the right operand
    expr.right->accept(*this);
    
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
        case TokenType::KW_IN:
            emitInstruction(OpCode::CONTAINS);
            break;
        default:
            error("Unknown binary operator: " + expr.op.lexeme, expr.op.location);
            break;
    }
}

void CodeGenerator::visitCallExpr(CallExpr& expr) {
    // Generate bytecode for the callee first (function object)
    expr.callee->accept(*this);
    
    // Generate bytecode for the arguments
    for (const auto& argument : expr.arguments) {
        argument->accept(*this);
    }
    
    // Emit a CALL instruction with the argument count
    // Check if arguments size fits in uint32_t to avoid truncation
    if (expr.arguments.size() > std::numeric_limits<uint32_t>::max()) {
        error("Too many arguments", expr.location);
        return;
    }
    // If callee is a member access, include the receiver as an implicit first argument
    uint32_t argc = static_cast<uint32_t>(expr.arguments.size());
    if (expr.callee && expr.callee->getType() == ExprType::MEMBER_ACCESS) {
        // Safe to increment; we already checked size upper bound above for arguments
        // Member-access calls treat the object as the first argument for native methods
        argc += 1;
    }
    emitInstruction(OpCode::CALL, argc);
}

void CodeGenerator::visitUnaryExpr(UnaryExpr& expr) {
    // Emit the appropriate unary operation instruction
    if (expr.op == "-") {
        // For unary minus, we need to push 0 and then perform subtraction: 0 - operand
        // First, load 0
        emitInstruction(OpCode::LOAD_CONST, static_cast<uint32_t>(currentChunk_->addConstant(Value(static_cast<int64_t>(0)))));
        // Then, generate bytecode for the operand
        expr.operand->accept(*this);
        // Finally, perform subtraction
        emitInstruction(OpCode::SUB);
    } else if (expr.op == "not") {
        // Generate bytecode for the operand
        expr.operand->accept(*this);
        emitInstruction(OpCode::NOT);
    } else {
        error("Unknown unary operator: " + expr.op, expr.location);
    }
}

void CodeGenerator::visitListExpr(ListExpr& expr) {
    // For each element in the list, generate bytecode to evaluate it
    for (const auto& element : expr.elements) {
        element->accept(*this);
    }
    
    // Check if elements size fits in uint32_t to avoid truncation
    if (expr.elements.size() > std::numeric_limits<uint32_t>::max()) {
        error("Too many elements in list", expr.location);
        return;
    }
    
    // Emit a BUILD_LIST instruction with the number of elements
    emitInstruction(OpCode::BUILD_LIST, static_cast<uint32_t>(expr.elements.size()));
}

void CodeGenerator::visitDictExpr(DictExpr& expr) {
    // For each key-value pair in the dictionary, generate bytecode
    // We need to evaluate both keys and values and push them in alternating order
    for (size_t i = 0; i < expr.keys.size(); ++i) {
        // Evaluate key
        expr.keys[i]->accept(*this);
        // Evaluate value
        expr.values[i]->accept(*this);
    }
    
    // Check if pairs size fits in uint32_t to avoid truncation
    if (expr.keys.size() > std::numeric_limits<uint32_t>::max()) {
        error("Too many key-value pairs in dictionary", expr.location);
        return;
    }
    
    // Emit a BUILD_DICT instruction with the number of key-value pairs
    emitInstruction(OpCode::BUILD_DICT, static_cast<uint32_t>(expr.keys.size()));
}

void CodeGenerator::visitTupleExpr(TupleExpr& expr) {
    // For each element in the tuple, generate bytecode to evaluate it
    for (const auto& element : expr.elements) {
        element->accept(*this);
    }
    
    // Check if elements size fits in uint32_t to avoid truncation
    if (expr.elements.size() > std::numeric_limits<uint32_t>::max()) {
        error("Too many elements in tuple", expr.location);
        return;
    }
    
    // Emit a BUILD_TUPLE instruction with the number of elements
    emitInstruction(OpCode::BUILD_TUPLE, static_cast<uint32_t>(expr.elements.size()));
}

void CodeGenerator::visitSetExpr(SetExpr& expr) {
    // For each element in the set, generate bytecode to evaluate it
    for (const auto& element : expr.elements) {
        element->accept(*this);
    }
    
    // Check if elements size fits in uint32_t to avoid truncation
    if (expr.elements.size() > std::numeric_limits<uint32_t>::max()) {
        error("Too many elements in set", expr.location);
        return;
    }
    
    // Emit a BUILD_SET instruction with the number of elements
    emitInstruction(OpCode::BUILD_SET, static_cast<uint32_t>(expr.elements.size()));
}

void CodeGenerator::visitMemberAccessExpr(MemberAccessExpr& expr) {
    // Generate bytecode for the object expression first
    expr.object->accept(*this);
    
    // Emit a LOAD_CONST instruction with the member name as a string
    Value memberName(expr.member);
    size_t memberIndex = emitConstant(memberName);
    
    // Check if memberIndex fits in uint32_t to avoid truncation
    if (memberIndex > std::numeric_limits<uint32_t>::max()) {
        error("Member name constant index too large", expr.location);
        return;
    }
    emitInstruction(OpCode::LOAD_CONST, static_cast<uint32_t>(memberIndex));
    
    // Emit a GET_ATTR instruction to get the attribute from the object
    emitInstruction(OpCode::GET_ATTR);
}

void CodeGenerator::visitIndexAccessExpr(IndexAccessExpr& expr) {
    // Generate bytecode for the object expression first
    expr.object->accept(*this);
    
    // Generate bytecode for the index expression
    expr.index->accept(*this);
    
    // Emit a GET_ITEM instruction to get the element at the specified index
    emitInstruction(OpCode::GET_ITEM);
}

// Statement visitors
void CodeGenerator::visitExprStmt(ExprStmt& stmt) {
    // Generate bytecode for the expression
    stmt.expression->accept(*this);
    
    // Pop the result if it's not used
    emitInstruction(OpCode::POP);
}

void CodeGenerator::visitBlockStmt(BlockStmt& stmt) {
    // Handle module-level docstring at top-level (in __main__)
    size_t startIndex = 0;
    if (currentFunction_ && currentFunction_->getName() == "__main__" && scopeDepth_ == 0) {
        if (!stmt.statements.empty()) {
            if (auto exprStmt = dynamic_cast<ExprStmt*>(stmt.statements[0].get())) {
                if (exprStmt->expression) {
                    if (auto litExpr = dynamic_cast<LiteralExpr*>(exprStmt->expression.get())) {
                        if (litExpr->token.type == TokenType::STRING) {
                            // Create and store module docstring in global __doc__ variable
                            size_t constIndex = emitConstant(Value(litExpr->token.lexeme));
                            if (constIndex > std::numeric_limits<uint32_t>::max()) {
                                error("Docstring constant index too large", stmt.location);
                                return;
                            }
                            emitInstruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constIndex));
                            size_t docVarIndex = createVariable("__doc__");
                            if (docVarIndex > std::numeric_limits<uint32_t>::max()) {
                                error("Variable index too large", stmt.location);
                                return;
                            }
                            emitInstruction(OpCode::STORE_VAR, static_cast<uint32_t>(docVarIndex));
                            // Discard the value from stack
                            emitInstruction(OpCode::POP);
                            // Also set the docstring on __main__ function object
                            currentFunction_->setDocstring(litExpr->token.lexeme);
                            // Skip this first statement when generating the rest of the block
                            startIndex = 1;
                        }
                    }
                }
            }
        }
    }

    // Begin a new scope
    beginScope();

    // Generate bytecode for each statement in the block
    for (size_t i = startIndex; i < stmt.statements.size(); ++i) {
        stmt.statements[i]->accept(*this);
    }

    // End the scope
    endScope();
}

void CodeGenerator::visitIfStmt(IfStmt& stmt) {
    // Generate bytecode for the condition
    stmt.condition->accept(*this);
    
    // Emit a JUMP_IF_FALSE instruction with a placeholder address
    // JUMP_IF_FALSE will consume the condition value, so no need for POP
    size_t elseJump = currentChunk_->addInstruction(Instruction(OpCode::JUMP_IF_FALSE, 0));
    
    // Generate bytecode for the then branch
    stmt.thenBranch->accept(*this);
    
    // Emit a JUMP instruction to skip the else branch
    size_t endJump = currentChunk_->addInstruction(Instruction(OpCode::JUMP, 0));
    
    // Patch the else jump to jump to the else branch or end
    patchJump(elseJump);
    
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
    // JUMP_IF_FALSE will consume the condition value, so no need for POP
    size_t exitJump = currentChunk_->addInstruction(Instruction(OpCode::JUMP_IF_FALSE, 0));
    
    // Generate bytecode for the loop body
    stmt.body->accept(*this);
    
    // Emit a JUMP instruction to go back to the start of the loop
    // Calculate the relative offset from current position to loopStart
    // The offset should be negative to jump backwards
    // Correct calculation: target_position - (current_position + 1)
    // We add 1 to current_position because the VM will increment ip_ before executing the JUMP
    int64_t jumpOffset = static_cast<int64_t>(loopStart) - static_cast<int64_t>(currentChunk_->size() + 1);
    
    // Check if jumpOffset fits in int32_t to avoid truncation
    if (jumpOffset < std::numeric_limits<int32_t>::min() || jumpOffset > std::numeric_limits<int32_t>::max()) {
        error("Jump offset too large", stmt.location);
        return;
    }
    
    // Convert the signed offset to uint32_t for storage in the instruction
    // The VM will convert it back to signed when executing the jump
    uint32_t unsignedOffset = static_cast<uint32_t>(static_cast<int32_t>(jumpOffset));
    emitInstruction(OpCode::JUMP, unsignedOffset);
    
    // Patch the exit jump to jump to the end of the loop
    patchJump(exitJump);
}

void CodeGenerator::visitForStmt(ForStmt& stmt) {
    
    // Generate bytecode for the iterable expression
    stmt.iterable->accept(*this);
    
    // Create an iterator from the iterable
    emitInstruction(OpCode::CREATE_ITER);
    
    // Store the iterator in a temporary variable for later reloading
    // Since HAS_NEXT and GET_NEXT now pop the iterator, we need to save it
    std::string iteratorVarName = "__iter_" + stmt.variable;
    size_t iteratorVarIndex = createVariable(iteratorVarName);
    // Check if iteratorVarIndex fits in uint32_t to avoid truncation
    if (iteratorVarIndex > std::numeric_limits<uint32_t>::max()) {
        error("Iterator variable index too large", stmt.location);
        return;
    }
    emitInstruction(OpCode::STORE_VAR, static_cast<uint32_t>(iteratorVarIndex));
    
    // Remember the start of the loop (HAS_NEXT instruction)
    size_t loopStart = currentChunk_->size();
    
    // Reload the iterator for HAS_NEXT check
    emitInstruction(OpCode::LOAD_VAR, static_cast<uint32_t>(iteratorVarIndex));
    
    // Check if the iterator has next element
    emitInstruction(OpCode::HAS_NEXT);
    
    // Emit a JUMP_IF_FALSE instruction to exit the loop
    size_t exitJump = currentChunk_->addInstruction(Instruction(OpCode::JUMP_IF_FALSE, 0));
    
    // Reload the iterator for GET_NEXT
    emitInstruction(OpCode::LOAD_VAR, static_cast<uint32_t>(iteratorVarIndex));
    
    // Get the next element from the iterator
    emitInstruction(OpCode::GET_NEXT);
    
    // Store the next element in the loop variable
    size_t varIndex = createVariable(stmt.variable);
    // Check if varIndex fits in uint32_t to avoid truncation
    if (varIndex > std::numeric_limits<uint32_t>::max()) {
        error("Variable index too large", stmt.location);
        return;
    }
    emitInstruction(OpCode::STORE_VAR, static_cast<uint32_t>(varIndex));
    
    // Generate bytecode for the loop body
    stmt.body->accept(*this);
    
    // After loop body execution, we need to reload the iterator for the next iteration
    // Since HAS_NEXT and GET_NEXT now pop the iterator, we need to reload it from the temporary variable
    emitInstruction(OpCode::LOAD_VAR, static_cast<uint32_t>(iteratorVarIndex));
    
    // Emit a JUMP instruction to go back to the HAS_NEXT instruction
    // Calculate the relative offset from current position to loopStart
    // The offset should be negative to jump backwards
    // Correct calculation: target_position - current_position
    int64_t jumpOffset = static_cast<int64_t>(loopStart) - static_cast<int64_t>(currentChunk_->size());
    // Check if jumpOffset fits in int32_t to avoid truncation
    if (jumpOffset < std::numeric_limits<int32_t>::min() || jumpOffset > std::numeric_limits<int32_t>::max()) {
        error("Jump offset too large", stmt.location);
        return;
    }
    // Convert the signed offset to uint32_t for storage in the instruction
    // The VM will convert it back to signed when executing the jump
    uint32_t unsignedOffset = static_cast<uint32_t>(static_cast<int32_t>(jumpOffset));
    emitInstruction(OpCode::JUMP, unsignedOffset);
    
    // Patch the exit jump to jump to the end of the loop
    patchJump(exitJump);
    
    // Pop the iterator from the stack
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
    
    // Store the function in the functions vector
    size_t functionIndex = functions_.size();
    functions_.push_back(function);
    
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
    
    // Prologue: bind parameters from stack to variables (reverse order)
    // Stack layout at function entry: [..., callee, arg1, arg2, ..., argN]
    // For each parameter from N down to 1: STORE_VAR (peek top as argN), POP
    if (!stmt.parameters.empty()) {
        for (int i = static_cast<int>(stmt.parameters.size()) - 1; i >= 0; --i) {
            const std::string& pname = stmt.parameters[i];
            size_t pIndex = getVariableIndex(pname);
            // Check if pIndex fits in uint32_t
            if (pIndex > std::numeric_limits<uint32_t>::max()) {
                error("Variable index too large", stmt.location);
                return;
            }
            emitInstruction(OpCode::STORE_VAR, static_cast<uint32_t>(pIndex));
            emitInstruction(OpCode::POP);
        }
    }
    // Pop the callee function object after binding all parameters
    emitInstruction(OpCode::POP);
    
    // Generate bytecode for the function body, handling an optional leading docstring
    bool skippedDocstring = false;
    if (stmt.body && !stmt.body->statements.empty()) {
        // Check first statement: if it's a string literal expression, treat as docstring
        if (auto exprStmt = dynamic_cast<ExprStmt*>(stmt.body->statements[0].get())) {
            if (exprStmt->expression) {
                if (auto litExpr = dynamic_cast<LiteralExpr*>(exprStmt->expression.get())) {
                    if (litExpr->token.type == TokenType::STRING) {
                        // Store docstring in the Function object
                        function->setDocstring(litExpr->token.lexeme);
                        skippedDocstring = true;
                    }
                }
            }
        }
    }

    // Manually visit body statements, skipping the docstring if present
    if (stmt.body) {
        beginScope();
        size_t startIndex = skippedDocstring ? 1 : 0;
        for (size_t i = startIndex; i < stmt.body->statements.size(); ++i) {
            stmt.body->statements[i]->accept(*this);
        }
        endScope();
    }
    
    // Emit a RETURN instruction if the function doesn't end with one
    if (stmt.body->statements.empty() || 
        (stmt.body->statements.back()->getType() != StmtType::RETURN_STMT)) {
        emitInstruction(OpCode::RETURN);
    }
    
    // End the scope (function-level)
    endScope();
    
    // Restore the previous function and chunk
    currentFunction_ = previousFunction;
    currentChunk_ = previousChunk;
    
    // Emit a constant for the function index (store the actual function index)
    size_t constIndex = emitConstant(Value(static_cast<uint32_t>(functionIndex), ValueType::FUNCTION));
    
    // Create a variable for the function name
    size_t varIndex = createVariable(stmt.name);
    
    // Emit a LOAD_CONST instruction with the function
    // Check if constIndex fits in uint32_t to avoid truncation
    if (constIndex > std::numeric_limits<uint32_t>::max()) {
        error("Function index too large", stmt.location);
        return;
    }
    emitInstruction(OpCode::LOAD_CONST, static_cast<uint32_t>(constIndex));
    
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
        size_t index = currentChunk_->addConstant(value);
        return index;
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
    
    // If we have a VM instance, set the variable name mapping
    // This will be used for error reporting
    if (vm_) {
        vm_->setVariableName(static_cast<uint32_t>(info.index), name);
    }
    
    return info.index;
}

bool CodeGenerator::isBuiltinFunction(const std::string& name) {
    // List of builtin functions
    static const std::unordered_set<std::string> builtinFunctions = {
        // I/O and core
        "print", "len", "any", "all",
        // Type checking
        "type", "isnil", "isboolean", "isinteger", "isfloat", "isnumber", "isstring", "islist", "isdict", "isfunction",
        // Math
        "abs", "min", "max", "sum",
        // Conversion and string
        "int", "str", "substr",
        // List
        "append", "remove", "extend", "insert", "pop", "clear", "sort", "reverse", "sorted", "reversed", "count", "index", "list_copy",
        // Dict
        "keys", "values", "contains", "update", "get", "copy", "fromkeys", "items", "dict_pop", "popitem", "setdefault"
    };
    
    return builtinFunctions.find(name) != builtinFunctions.end();
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

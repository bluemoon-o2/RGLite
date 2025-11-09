// RGLite Semantic Analyzer Implementation
// This file implements the semantic analyzer for RGLite

#include "SemanticAnalyzer.h"
#include <memory>
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <unordered_set>

namespace rglite {

// Scope implementation
Scope::Scope(std::shared_ptr<Scope> parent) : parent_(parent) {
}

bool Scope::define(const std::string& name, std::unique_ptr<Symbol> symbol) {
    if (symbols_.find(name) != symbols_.end()) {
        return false; // Symbol already defined in this scope
    }
    
    symbols_[name] = std::move(symbol);
    return true;
}

Symbol* Scope::resolve(const std::string& name) const {
    auto it = symbols_.find(name);
    if (it != symbols_.end()) {
        return it->second.get();
    }
    
    // Check parent scope recursively
    if (parent_) {
        return parent_->resolve(name);
    }
    
    return nullptr;
}



// SymbolTable implementation
SymbolTable::SymbolTable() {
    // Initialize with global scope
    globalScope_ = std::make_shared<Scope>();
    currentScope_ = globalScope_;
    
    // Define built-in functions
    defineBuiltins();
}

void SymbolTable::pushScope() {
    auto newScope = std::make_shared<Scope>(currentScope_);
    currentScope_ = newScope;
}

void SymbolTable::popScope() {
    if (currentScope_ != globalScope_) {
        currentScope_ = currentScope_->getParent();
    }
}

bool SymbolTable::define(const std::string& name, std::unique_ptr<Symbol> symbol) {
    return currentScope_->define(name, std::move(symbol));
}

Symbol* SymbolTable::resolve(const std::string& name) const {
    // Use the current scope's resolve method which will search through all parent scopes
    return currentScope_->resolve(name);
}

Symbol* SymbolTable::lookup(const std::string& name) const {
    return resolve(name);
}

void SymbolTable::defineBuiltins() {
    // Define built-in functions (synchronized with VM::registerBuiltinFunctions)
    // I/O and core
    define("print", std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "print", SourceLocation()));
    define("len",   std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "len",   SourceLocation()));

    // Type checking
    define("type",       std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "type",       SourceLocation()));
    define("isnil",      std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "isnil",      SourceLocation()));
    define("isboolean",  std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "isboolean",  SourceLocation()));
    define("isinteger",  std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "isinteger",  SourceLocation()));
    define("isfloat",    std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "isfloat",    SourceLocation()));
    define("isnumber",   std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "isnumber",   SourceLocation()));
    define("isstring",   std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "isstring",   SourceLocation()));
    define("islist",     std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "islist",     SourceLocation()));
    define("isdict",     std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "isdict",     SourceLocation()));
    define("isfunction", std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "isfunction", SourceLocation()));

    // Math
    define("abs", std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "abs", SourceLocation()));
    define("min", std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "min", SourceLocation()));
    define("max", std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "max", SourceLocation()));

    // Conversion and string
    define("int",    std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "int",    SourceLocation()));
    define("str",    std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "str",    SourceLocation()));
    define("substr", std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "substr", SourceLocation()));

    // List functions
    define("append",    std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "append",    SourceLocation()));
    define("remove",    std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "remove",    SourceLocation()));
    define("extend",    std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "extend",    SourceLocation()));
    define("insert",    std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "insert",    SourceLocation()));
    define("pop",       std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "pop",       SourceLocation()));
    define("clear",     std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "clear",     SourceLocation()));
    define("sort",      std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "sort",      SourceLocation()));
    define("reverse",   std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "reverse",   SourceLocation()));
    define("count",     std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "count",     SourceLocation()));
    define("index",     std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "index",     SourceLocation()));
    define("list_copy", std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "list_copy", SourceLocation()));

    // Dict functions
    define("keys",       std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "keys",       SourceLocation()));
    define("values",     std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "values",     SourceLocation()));
    define("contains",   std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "contains",   SourceLocation()));
    define("update",     std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "update",     SourceLocation()));
    define("get",        std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "get",        SourceLocation()));
    define("copy",       std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "copy",       SourceLocation()));
    define("fromkeys",   std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "fromkeys",   SourceLocation()));
    define("items",      std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "items",      SourceLocation()));
    define("dict_pop",   std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "dict_pop",   SourceLocation()));
    define("popitem",    std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "popitem",    SourceLocation()));
    define("setdefault", std::make_unique<Symbol>(Symbol::Kind::BUILTIN, "setdefault", SourceLocation()));
}

// TypeChecker implementation
TypeChecker::TypeChecker(std::shared_ptr<ErrorHandler> errorHandler) 
    : errorHandler_(errorHandler) {
    if (!errorHandler_) {
        errorHandler_ = std::make_shared<StandardErrorHandler>();
    }
}

Type TypeChecker::check(const std::unique_ptr<Expr>& expr) {
    // This is a placeholder implementation
    // In a full implementation, we would dispatch to specific type checking methods
    // based on the expression type
    
    if (!expr) {
        return Type::UNKNOWN;
    }
    
    // For now, return UNKNOWN for all expressions
    // This will be implemented in the full SemanticAnalyzer
    return Type::UNKNOWN;
}

Type TypeChecker::checkBinaryOperation(Type left, Type right, const std::string& op) {
    // Arithmetic operations
    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
        if (isNumeric(left) && isNumeric(right)) {
            // If both operands are integers, result is integer
            // If either operand is float, result is float
            return (left == Type::FLOAT || right == Type::FLOAT) ? Type::FLOAT : Type::INTEGER;
        }
        
        // String concatenation
        if (op == "+" && left == Type::STRING && right == Type::STRING) {
            return Type::STRING;
        }
        
        // Invalid string operations
        if ((left == Type::STRING || right == Type::STRING) && op != "+") {
            return Type::UNKNOWN;
        }
        
        return Type::UNKNOWN;
    }
    
    // Comparison operations
    if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") {
        // All comparison operations return boolean
        // Type compatibility will be checked elsewhere
        return Type::BOOLEAN;
    }
    
    // Logical operations
    if (op == "and" || op == "or") {
        // Logical operations return boolean
        // Operands must be boolean or convertible to boolean
        return Type::BOOLEAN;
    }
    
    // Assignment
    if (op == "=") {
        // Assignment returns the type of the right operand
        return right;
    }
    
    return Type::UNKNOWN;
}

Type TypeChecker::checkUnaryOperation(Type operand, const std::string& op) {
    if (op == "-") {
        if (isNumeric(operand)) {
            return operand;
        }
        return Type::UNKNOWN;
    }
    
    if (op == "not") {
        // Logical not returns boolean
        return Type::BOOLEAN;
    }
    
    return Type::UNKNOWN;
}

Type TypeChecker::checkFunctionCall(const std::string& name, const std::vector<Type>& argTypes) {
    // Built-in functions
    if (name == "print") {
        return Type::NONE;
    }
    
    if (name == "input") {
        if (argTypes.size() == 0 || (argTypes.size() == 1 && argTypes[0] == Type::STRING)) {
            return Type::STRING;
        }
        return Type::UNKNOWN;
    }
    
    if (name == "len") {
        if (argTypes.size() != 1) {
            return Type::UNKNOWN;
        }
        if (argTypes[0] != Type::STRING) {
            return Type::UNKNOWN;
        }
        return Type::INTEGER;
    }
    
    if (name == "int") {
        if (argTypes.size() != 1) {
            return Type::UNKNOWN;
        }
        return Type::INTEGER;
    }
    
    if (name == "float") {
        if (argTypes.size() != 1) {
            return Type::UNKNOWN;
        }
        return Type::FLOAT;
    }
    
    if (name == "str") {
        if (argTypes.size() != 1) {
            return Type::UNKNOWN;
        }
        return Type::STRING;
    }
    
    if (name == "has_next") {
        if (argTypes.size() != 1) {
            return Type::UNKNOWN;
        }
        // has_next can be applied to lists and dicts
        if (argTypes[0] == Type::LIST || argTypes[0] == Type::DICT) {
            return Type::BOOLEAN;
        }
        return Type::UNKNOWN;
    }
    
    if (name == "next") {
        if (argTypes.size() != 1) {
            return Type::UNKNOWN;
        }
        // next can be applied to lists and dicts
        if (argTypes[0] == Type::LIST || argTypes[0] == Type::DICT) {
            return Type::UNKNOWN; // Return type depends on the collection type
        }
        return Type::UNKNOWN;
    }
    
    // User-defined functions would be checked here
    return Type::UNKNOWN;
}

Type TypeChecker::checkFunctionCall(Symbol* symbol, const std::vector<std::unique_ptr<Expr>>& args) {
    if (!symbol) {
        return Type::UNKNOWN;
    }
    
    // For built-in functions, do some basic type checking
    if (symbol->kind == Symbol::Kind::BUILTIN) {
        if (symbol->name == "print") {
            // print can take any type
            return Type::NONE;
        }
        
        if (symbol->name == "input") {
            // input takes an optional string argument and returns a string
            if (args.size() == 0 || args.size() == 1) {
                return Type::STRING;
            }
            return Type::UNKNOWN;
        }
        
        if (symbol->name == "len") {
            // len takes one argument of type string, list, etc.
            if (args.size() == 1) {
                return Type::INTEGER;
            }
            return Type::UNKNOWN;
        }
        
        if (symbol->name == "has_next") {
            // has_next takes one argument of type list or dict
            if (args.size() == 1) {
                return Type::BOOLEAN;
            }
            return Type::UNKNOWN;
        }
        
        if (symbol->name == "next") {
            // next takes one argument of type list or dict
            if (args.size() == 1) {
                return Type::UNKNOWN; // Return type depends on the collection type
            }
            return Type::UNKNOWN;
        }
    }
    
    // For other functions, return UNKNOWN for now
    return Type::UNKNOWN;
}

Type TypeChecker::checkIndexAccess(Type objectType, Type indexType) {
    // Check if the object is a list or dictionary
    if (objectType != Type::LIST && objectType != Type::DICT && objectType != Type::UNKNOWN) {
        return Type::UNKNOWN;
    }
    
    // For lists, index must be an integer
    if (objectType == Type::LIST) {
        if (indexType != Type::INTEGER && indexType != Type::UNKNOWN) {
            return Type::UNKNOWN;
        }
        // Return unknown type for list elements (they can be any type)
        return Type::UNKNOWN;
    }
    
    // For dictionaries, index must be a string or integer
    if (objectType == Type::DICT) {
        if (indexType != Type::STRING && indexType != Type::INTEGER && indexType != Type::UNKNOWN) {
            return Type::UNKNOWN;
        }
        // Return unknown type for dictionary values (they can be any type)
        return Type::UNKNOWN;
    }
    
    return Type::UNKNOWN;
}

std::string TypeChecker::typeToString(Type type) const {
    switch (type) {
        case Type::INTEGER: return "integer";
        case Type::FLOAT: return "float";
        case Type::STRING: return "string";
        case Type::BOOLEAN: return "boolean";
        case Type::NONE: return "none";
        case Type::FUNCTION: return "function";
        case Type::LIST: return "list";
        case Type::DICT: return "dict";
        case Type::UNKNOWN: return "unknown";
        default: return "unknown";
    }
}

bool TypeChecker::isAssignable(Type type) const {
    // All types except UNKNOWN are assignable
    return type != Type::UNKNOWN;
}

bool TypeChecker::isNumeric(Type type) const {
    return type == Type::INTEGER || type == Type::FLOAT;
}

bool TypeChecker::isCollection(Type type) const {
    return type == Type::LIST || type == Type::DICT;
}

// SemanticAnalyzer implementation
SemanticAnalyzer::SemanticAnalyzer(std::shared_ptr<ErrorHandler> errorHandler) 
    : errorHandler_(errorHandler) {
    
    if (!errorHandler_) {
        errorHandler_ = std::make_shared<StandardErrorHandler>();
    }
    
    symbolTable_ = std::make_shared<SymbolTable>();
    typeChecker_ = std::make_unique<TypeChecker>(errorHandler_);
}

bool SemanticAnalyzer::analyze(const std::unique_ptr<Stmt>& ast) {
    if (!ast) {
        return true;
    }
    
    hasErrors_ = false;
    
    try {
        // Handle both single statements and block statements
        if (ast->getType() == StmtType::BLOCK) {
            auto blockStmt = dynamic_cast<BlockStmt*>(ast.get());
            if (blockStmt) {
                for (const auto& stmt : blockStmt->statements) {
                    analyzeStatement(stmt);
                }
            }
        } else {
            // For single statements, directly analyze them
            analyzeStatement(ast);
        }
    } catch (const std::exception&) {
        hasErrors_ = true;
    }
    
    return !hasErrors_;
}

bool SemanticAnalyzer::hasErrors() const {
    return hasErrors_;
}

void SemanticAnalyzer::error(const std::string& message, const SourceLocation& location) {
    hasErrors_ = true;
    // Fill source line and column for accurate caret
    std::string sline = getSourceLine(static_cast<int>(location.line));
    int col = static_cast<int>(location.column);
    std::string fname = filename_.empty() ? std::string("<stdin>") : filename_;
    Diagnostic diagnostic(Severity::ERROR, message, location, ErrorCode::TYPE_MISMATCH,
                         sline, col, 0, 0, fname, getCurrentFunction());
    errorHandler_->report(diagnostic);
}

void SemanticAnalyzer::error(const SourceLocation& location, const std::string& message) {
    hasErrors_ = true;
    // Fill source line and column for accurate caret
    std::string sline = getSourceLine(static_cast<int>(location.line));
    int col = static_cast<int>(location.column);
    std::string fname = filename_.empty() ? std::string("<stdin>") : filename_;
    Diagnostic diagnostic(Severity::ERROR, message, location, ErrorCode::TYPE_MISMATCH,
                         sline, col, 0, 0, fname, getCurrentFunction());
    errorHandler_->report(diagnostic);
}

void SemanticAnalyzer::analyzeStatement(const std::unique_ptr<Stmt>& stmt) {
    if (!stmt) {
        return;
    }
    
    switch (stmt->getType()) {
        case StmtType::BLOCK:
            analyzeBlockStmt(dynamic_cast<BlockStmt*>(stmt.get()));
            break;
        case StmtType::EXPR_STMT:
            analyzeExprStmt(dynamic_cast<ExprStmt*>(stmt.get()));
            break;

        case StmtType::FUNC_DECL:
            analyzeFunctionDeclStmt(dynamic_cast<FunctionDeclStmt*>(stmt.get()));
            break;
        case StmtType::IF_STMT:
            analyzeIfStmt(dynamic_cast<IfStmt*>(stmt.get()));
            break;
        case StmtType::WHILE_STMT:
            analyzeWhileStmt(dynamic_cast<WhileStmt*>(stmt.get()));
            break;
        case StmtType::FOR_STMT:
            analyzeForStmt(dynamic_cast<ForStmt*>(stmt.get()));
            break;
        case StmtType::RETURN_STMT:
            analyzeReturnStmt(dynamic_cast<ReturnStmt*>(stmt.get()));
            break;
    }
}

void SemanticAnalyzer::analyzeBlockStmt(BlockStmt* stmt) {
    if (!stmt) {
        return;
    }
    
    // Enter new scope
    symbolTable_->pushScope();
    
    // Analyze statements in the block
    for (size_t i = 0; i < stmt->statements.size(); i++) {
        analyzeStatement(stmt->statements[i]);
    }
    
    // Exit scope - but don't exit function state here
    // Function state is managed by analyzeFunctionDeclStmt
    symbolTable_->popScope();
}

void SemanticAnalyzer::analyzeExprStmt(ExprStmt* stmt) {
    // In Python-style, all assignments are treated as expression statements
    // Variable declaration is implicit through assignment
    analyzeExpression(stmt->expression);
}



void SemanticAnalyzer::analyzeFunctionDeclStmt(FunctionDeclStmt* stmt) {
    
    // Check if function is already defined in current scope
    auto currentScope = symbolTable_->getCurrentScope();
    if (currentScope) {
        auto symbol = currentScope->resolve(stmt->name);
        if (symbol) {
            if (symbol->kind == Symbol::Kind::FUNCTION) {
                error(stmt->location, "Function '" + stmt->name + "' is already defined in this scope");
            } else {
                error(stmt->location, "'" + stmt->name + "' is already defined as a variable in this scope");
            }
            return;
        }
    }
    
    // Check for duplicate parameter names
    std::unordered_set<std::string> paramNames;
    for (const auto& param : stmt->parameters) {
        if (paramNames.find(param) != paramNames.end()) {
            error(stmt->location, "Parameter '" + param + "' is already defined in this function");
            return;
        }
        paramNames.insert(param);
    }
    
    // Create function symbol
    auto symbol = std::make_unique<Symbol>(Symbol::Kind::FUNCTION, stmt->name, stmt->location);
    symbol->parameters = stmt->parameters;
    
    // Add to symbol table in the current scope (parent scope)
    if (!symbolTable_->define(stmt->name, std::move(symbol))) {
        error(stmt->location, "Failed to define function '" + stmt->name + "'");
        return;
    }
    
    // Save current state before entering function
    std::string previousFunction = getCurrentFunction();
    
    // Create a new scope for the function body
    symbolTable_->pushScope();
    
    // Enter function scope
    enterFunction(stmt->name, stmt->parameters);
    
    // Define parameters in the new scope
    for (const auto& param : stmt->parameters) {
        auto paramSymbol = std::make_unique<Symbol>(Symbol::Kind::PARAMETER, param, stmt->location);
        paramSymbol->isInitialized = true;
        paramSymbol->type = Type::UNKNOWN;
        
        if (!symbolTable_->define(param, std::move(paramSymbol))) {
            error(stmt->location, "Failed to define parameter '" + param + "'");
        }
    }
    
    // Enter new scope for function body
    symbolTable_->pushScope();
    
    // Use analyzeBlockStmt to analyze the function body
    // This ensures proper handling of nested scopes and function state
    analyzeBlockStmt(stmt->body.get());
    
    // Exit function scope AFTER analyzing all statements in the function body
    // This ensures that return statements within the function body are correctly analyzed
    exitFunction();
    
    // Pop the scope for the function body
    symbolTable_->popScope();
    
    // Pop the scope for the parameters
    symbolTable_->popScope();
}

void SemanticAnalyzer::analyzeIfStmt(IfStmt* stmt) {
    // Analyze condition
    Type conditionType = analyzeExpression(stmt->condition);
    
    // Condition must be boolean or convertible to boolean
    if (conditionType != Type::BOOLEAN && conditionType != Type::UNKNOWN) {
        error(stmt->condition->getLocation(), "If condition must be a boolean expression");
    }
    
    // Analyze then branch
    analyzeStatement(stmt->thenBranch);
    
    // Analyze else branch if present
    if (stmt->elseBranch) {
        analyzeStatement(stmt->elseBranch);
    }
}

void SemanticAnalyzer::analyzeWhileStmt(WhileStmt* stmt) {
    // Analyze condition
    Type conditionType = analyzeExpression(stmt->condition);
    
    // Condition must be boolean or convertible to boolean
    if (conditionType != Type::BOOLEAN && conditionType != Type::UNKNOWN) {
        error(stmt->condition->getLocation(), "While condition must be a boolean expression");
    }
    
    // Analyze body
    analyzeStatement(stmt->body);
}

void SemanticAnalyzer::analyzeForStmt(ForStmt* stmt) {
    // Analyze iterable expression
    Type iterableType = analyzeExpression(stmt->iterable);
    
    // Iterable must be a list or dictionary
    if (iterableType != Type::LIST && iterableType != Type::DICT && iterableType != Type::UNKNOWN) {
        error(stmt->iterable->getLocation(), "For loop iterable must be a list or dictionary");
    }
    
    // Enter a new scope for the loop variable
    symbolTable_->pushScope();
    
    // Declare the loop variable in the current scope
    // The type of the loop variable depends on the iterable type
    Type variableType = Type::UNKNOWN;
    if (iterableType == Type::LIST) {
        variableType = Type::UNKNOWN; // List elements can be of any type
    } else if (iterableType == Type::DICT) {
        variableType = Type::STRING; // Dictionary keys are strings
    }
    
    auto loopVarSymbol = std::make_unique<Symbol>(Symbol::Kind::VARIABLE, stmt->variable, stmt->location);
    loopVarSymbol->isInitialized = true;
    
    if (!symbolTable_->define(stmt->variable, std::move(loopVarSymbol))) {
        error(stmt->location, "Failed to define loop variable '" + stmt->variable + "'");
    }
    
    // Analyze the loop body
    analyzeStatement(stmt->body);
    
    // Exit the loop variable scope
    symbolTable_->popScope();
}

void SemanticAnalyzer::analyzeReturnStmt(ReturnStmt* stmt) {
    if (!isInFunction()) {
        // Match CPython wording
        error(stmt->location, "'return' outside function");
        return;
    }
    
    // Analyze return value if present
    if (stmt->value) {
        analyzeExpression(stmt->value);
    }
}

Type SemanticAnalyzer::analyzeExpression(const std::unique_ptr<Expr>& expr) {
    if (!expr) {
        return Type::UNKNOWN;
    }
    
    switch (expr->getType()) {
        case ExprType::BINARY:
            return analyzeBinaryExpr(dynamic_cast<BinaryExpr*>(expr.get()));
        case ExprType::UNARY:
            return analyzeUnaryExpr(dynamic_cast<UnaryExpr*>(expr.get()));
        case ExprType::LITERAL:
            return analyzeLiteralExpr(dynamic_cast<LiteralExpr*>(expr.get()));
        case ExprType::IDENTIFIER: {
            auto symbol = analyzeIdentifierExpr(dynamic_cast<IdentifierExpr*>(expr.get()));
            if (!symbol) {
                // analyzeIdentifierExpr already reported the error, just return UNKNOWN
                return Type::UNKNOWN;
            }
            
            // Return appropriate type based on symbol kind
            switch (symbol->kind) {
                case Symbol::Kind::VARIABLE:
                case Symbol::Kind::PARAMETER:
                    // Return tracked type for variables/parameters
                    return symbol->type;
                case Symbol::Kind::FUNCTION:
                case Symbol::Kind::BUILTIN:
                    // Functions return UNKNOWN type for now
                    return Type::UNKNOWN;
                default:
                    return Type::UNKNOWN;
            }
        }
        case ExprType::CALL:
            return analyzeCallExpr(dynamic_cast<CallExpr*>(expr.get()));
        case ExprType::LIST:
            return analyzeListExpr(dynamic_cast<ListExpr*>(expr.get()));
        case ExprType::DICT:
            return analyzeDictExpr(dynamic_cast<DictExpr*>(expr.get()));
        case ExprType::INDEX_ACCESS:
            return analyzeIndexAccessExpr(dynamic_cast<IndexAccessExpr*>(expr.get()));
        default:
            return Type::UNKNOWN;
    }
}

Type SemanticAnalyzer::analyzeBinaryExpr(BinaryExpr* expr) {
    if (!expr) {
        return Type::UNKNOWN;
    }
    
    // For assignment operations, handle variable creation/update first
    if (expr->op.lexeme == "=") {
        // Check if left operand is an identifier (valid assignment target)
        if (expr->left->getType() != ExprType::IDENTIFIER) {
            error(expr->op.location, "Invalid assignment target");
            return Type::UNKNOWN;
        }
        
        auto identifier = dynamic_cast<IdentifierExpr*>(expr->left.get());
        if (!identifier) {
            error(expr->op.location, "Invalid assignment target");
            return Type::UNKNOWN;
        }
        
        // Analyze right side first to get its type
        Type rightType = analyzeExpression(expr->right);
        
        // Check if variable is already defined
        auto symbol = symbolTable_->resolve(identifier->name);
        if (symbol) {
            // Variable already exists, check if it's a valid assignment target
            if (symbol->kind != Symbol::Kind::VARIABLE) {
                if (symbol->kind == Symbol::Kind::FUNCTION) {
                    error(expr->op.location, "'" + identifier->name + "' is a function, not a variable");
                } else {
                    error(expr->op.location, "'" + identifier->name + "' is a parameter, not a variable");
                }
                return Type::UNKNOWN;
            }
            // Mark as initialized
            symbol->isInitialized = true;
            symbol->type = rightType;
        } else {
            // Variable doesn't exist, create it (implicit variable declaration)
            auto newSymbol = std::make_unique<Symbol>(Symbol::Kind::VARIABLE, identifier->name, expr->op.location);
            newSymbol->isInitialized = true;
            newSymbol->type = rightType;
            
            if (!symbolTable_->define(identifier->name, std::move(newSymbol))) {
                error(expr->op.location, "Failed to define variable '" + identifier->name + "'");
                return Type::UNKNOWN;
            }
        }
        
        // For assignment operations, we need to check if the right side contains undefined variables
        // This is where we report errors for undefined variables in assignment expressions
        // But only check if we're not in a function body (to avoid reporting function parameters as undefined)
        // However, we should still check for undefined variables in the right side even in function body
        // because function parameters are already defined and shouldn't be reported as undefined
        checkForUndefinedVariables(expr->right);
        
        return rightType;
    }
    
    // For non-assignment operations, analyze both sides normally
    Type leftType = analyzeExpression(expr->left);
    Type rightType = analyzeExpression(expr->right);
    
    // For non-assignment operations, check if operands contain undefined variables
    // This is where we report errors for undefined variables in expressions
    // But only check if we're not in a function body (to avoid reporting function parameters as undefined)
    // However, we should still check for undefined variables even in function body
    // because function parameters are already defined and shouldn't be reported as undefined
    if (expr->op.lexeme != "=") {
        // Check if left operand is an identifier that couldn't be resolved
        // Now check unconditionally - function parameters are already defined and won't be reported as undefined
        if (expr->left->getType() == ExprType::IDENTIFIER) {
            auto leftIdentifier = dynamic_cast<IdentifierExpr*>(expr->left.get());
            if (leftIdentifier && !symbolTable_->resolve(leftIdentifier->name)) {
                error(leftIdentifier->location, "Undefined variable '" + leftIdentifier->name + "'");
            }
        }
        
        // Check if right operand is an identifier that couldn't be resolved
        // Now check unconditionally - function parameters are already defined and won't be reported as undefined
        if (expr->right->getType() == ExprType::IDENTIFIER) {
            auto rightIdentifier = dynamic_cast<IdentifierExpr*>(expr->right.get());
            if (rightIdentifier && !symbolTable_->resolve(rightIdentifier->name)) {
                error(rightIdentifier->location, "Undefined variable '" + rightIdentifier->name + "'");
            }
        }
    }
    
    // Check other binary operations
    Type resultType = typeChecker_->checkBinaryOperation(leftType, rightType, expr->op.lexeme);
    
    // Only report an error if both operand types are known and the result is invalid
    if (resultType == Type::UNKNOWN && leftType != Type::UNKNOWN && rightType != Type::UNKNOWN) {
        error(expr->op.location, "Invalid operands for binary operator '" + expr->op.lexeme + "'");
    }
    
    return resultType;
}

Type SemanticAnalyzer::analyzeUnaryExpr(UnaryExpr* expr) {
    if (!expr) {
        return Type::UNKNOWN;
    }
    
    Type operandType = analyzeExpression(expr->operand);
    
    Type resultType = typeChecker_->checkUnaryOperation(operandType, expr->op);
    
    // Only report an error if operand type is known and the result is invalid
    if (resultType == Type::UNKNOWN && operandType != Type::UNKNOWN) {
        error(expr->location, "Invalid operand for unary operator '" + expr->op + "'");
    }
    
    return resultType;
}

Type SemanticAnalyzer::analyzeLiteralExpr(LiteralExpr* expr) {
    if (!expr) {
        return Type::UNKNOWN;
    }
    
    switch (expr->token.type) {
        case TokenType::INTEGER:
            return Type::INTEGER;
        case TokenType::FLOAT:
            return Type::FLOAT;
        case TokenType::STRING:
            return Type::STRING;
        case TokenType::KW_TRUE:
        case TokenType::KW_FALSE:
            return Type::BOOLEAN;
        case TokenType::KW_NONE:
            return Type::NONE;
        default:
            return Type::UNKNOWN;
    }
}

std::shared_ptr<Symbol> SemanticAnalyzer::analyzeIdentifierExpr(IdentifierExpr* expr) {
    if (!expr) {
        return nullptr;
    }
    
    // Look up the symbol in the current scope
    Symbol* symbol = symbolTable_->resolve(expr->name);
    if (!symbol) {
        // Now report undefined variable error unconditionally
        // Function parameters are already defined and won't be reported as undefined
        error(expr->location, "Undefined variable '" + expr->name + "'");
        return nullptr;
    }
    

    
    // Check if trying to use a function as a variable - this should be allowed in some contexts
    // Functions can be used as values in function calls, so we should not error here
    // The error should be in the context where the identifier is used
    
    // Check if variable is initialized before use
    if (symbol->kind == Symbol::Kind::VARIABLE && !symbol->isInitialized) {
        error(expr->location, "Variable '" + expr->name + "' is used before being initialized");
    }
    
    // Mark the symbol as used
    symbol->used = true;
    
    // Return a shared_ptr that doesn't own the symbol
    // Use a static no-op deleter function to avoid lambda issues
    static auto noOpDeleter = [](Symbol*) {};
    return std::shared_ptr<Symbol>(symbol, noOpDeleter);
}

Type SemanticAnalyzer::analyzeCallExpr(CallExpr* expr) {
    if (!expr) {
        return Type::UNKNOWN;
    }
    
    // Analyze arguments
    std::vector<Type> argTypes;
    for (const auto& arg : expr->arguments) {
        argTypes.push_back(analyzeExpression(arg));
    }
    
    // Check if callee is a function identifier
    if (expr->callee->getType() == ExprType::IDENTIFIER) {
        auto identifier = dynamic_cast<IdentifierExpr*>(expr->callee.get());
        if (identifier) {
            // Resolve symbol and accept either user-defined function or builtin function generically
            auto symbol = symbolTable_->resolve(identifier->name);
            if (!symbol) {
                error(expr->location, "Undefined function '" + identifier->name + "'");
                return Type::UNKNOWN;
            }

            if (symbol->kind != Symbol::Kind::FUNCTION && symbol->kind != Symbol::Kind::BUILTIN) {
                if (symbol->kind == Symbol::Kind::VARIABLE) {
                    error(expr->location, "'" + identifier->name + "' is a variable, not a function");
                } else {
                    error(expr->location, "'" + identifier->name + "' is a parameter, not a function");
                }
                return Type::UNKNOWN;
            }

            // Check parameter count for user-defined functions only
            if (symbol->kind == Symbol::Kind::FUNCTION) {
                if (argTypes.size() != symbol->parameters.size()) {
                    error(expr->location, "Function '" + identifier->name + "' expects " +
                          std::to_string(symbol->parameters.size()) + " arguments, got " +
                          std::to_string(argTypes.size()));
                    return Type::UNKNOWN;
                }
            }

            // Mark as used
            symbol->used = true;
            return Type::UNKNOWN; // For now, all function calls return UNKNOWN
        }
    }
    
    // If not an identifier, it's an invalid function call
    error(expr->location, "Invalid function call");
    return Type::UNKNOWN;
}

Type SemanticAnalyzer::analyzeListExpr(ListExpr* expr) {
    if (!expr) {
        return Type::UNKNOWN;
    }
    
    // Analyze all elements in the list
    for (const auto& element : expr->elements) {
        analyzeExpression(element);
    }
    
    // Lists can contain elements of any type
    return Type::LIST;
}

Type SemanticAnalyzer::analyzeDictExpr(DictExpr* expr) {
    if (!expr) {
        return Type::UNKNOWN;
    }
    
    // Check that keys and values have the same number
    if (expr->keys.size() != expr->values.size()) {
        error(expr->location, "Dictionary keys and values count mismatch");
        return Type::UNKNOWN;
    }
    
    // Analyze all keys and values
    for (const auto& key : expr->keys) {
        Type keyType = analyzeExpression(key);
        // Dictionary keys should be strings or integers
        if (keyType != Type::STRING && keyType != Type::INTEGER && keyType != Type::UNKNOWN) {
            error(key->getLocation(), "Dictionary keys must be strings or integers");
        }
    }
    
    for (const auto& value : expr->values) {
        analyzeExpression(value);
        // Dictionary values can be of any type
    }
    
    return Type::DICT;
}

Type SemanticAnalyzer::analyzeIndexAccessExpr(IndexAccessExpr* expr) {
    if (!expr) {
        return Type::UNKNOWN;
    }
    
    // Analyze the object being accessed
    Type objectType = analyzeExpression(expr->object);
    
    // Analyze the index expression
    Type indexType = analyzeExpression(expr->index);
    
    // Use TypeChecker to validate the index access
    Type resultType = typeChecker_->checkIndexAccess(objectType, indexType);
    
    // Check if the object is a list or dictionary
    if (objectType != Type::LIST && objectType != Type::DICT && objectType != Type::UNKNOWN) {
        error(expr->object->getLocation(), "Index access is only supported for lists and dictionaries");
        return Type::UNKNOWN;
    }
    
    // For lists, index must be an integer
    if (objectType == Type::LIST) {
        if (indexType != Type::INTEGER && indexType != Type::UNKNOWN) {
            error(expr->index->getLocation(), "List index must be an integer");
            return Type::UNKNOWN;
        }
        // Return unknown type for list elements (they can be any type)
        return Type::UNKNOWN;
    }
    
    // For dictionaries, index must be a string or integer
    if (objectType == Type::DICT) {
        if (indexType != Type::STRING && indexType != Type::INTEGER && indexType != Type::UNKNOWN) {
            error(expr->index->getLocation(), "Dictionary key must be a string or integer");
            return Type::UNKNOWN;
        }
        // Return unknown type for dictionary values (they can be any type)
        return Type::UNKNOWN;
    }
    
    return resultType;
}

void SemanticAnalyzer::enterFunction(const std::string& name, const std::vector<std::string>& /* parameters */) {
    // Save current function state
    functionStack_.push({inFunction_, currentFunction_});
    
    // Set new function state
    inFunction_ = true;
    currentFunction_ = name;
}

void SemanticAnalyzer::exitFunction() {
    if (!functionStack_.empty()) {
        // Restore previous function state
        auto [prevInFunction, prevFunction] = functionStack_.top();
        functionStack_.pop();
        
        inFunction_ = prevInFunction;
        currentFunction_ = prevFunction;
    }
}

void SemanticAnalyzer::checkForUndefinedVariables(const std::unique_ptr<Expr>& expr) {
    if (!expr) {
        return;
    }
    
    // Note: We now check for undefined variables even in function bodies
    // Function parameters are properly defined in the function scope, so they won't be reported as undefined
    
    switch (expr->getType()) {
        case ExprType::IDENTIFIER: {
            auto identifier = dynamic_cast<IdentifierExpr*>(expr.get());
            // Report undefined variables regardless of whether we're in a function body
            // Function parameters are properly defined and won't be reported as undefined
            if (identifier && !symbolTable_->resolve(identifier->name)) {
                error(identifier->location, "Undefined variable '" + identifier->name + "'");
            }
            break;
        }
        case ExprType::BINARY: {
            auto binaryExpr = dynamic_cast<BinaryExpr*>(expr.get());
            if (binaryExpr) {
                // For binary expressions, check both sides
                checkForUndefinedVariables(binaryExpr->left);
                checkForUndefinedVariables(binaryExpr->right);
            }
            break;
        }
        case ExprType::UNARY: {
            auto unaryExpr = dynamic_cast<UnaryExpr*>(expr.get());
            if (unaryExpr) {
                checkForUndefinedVariables(unaryExpr->operand);
            }
            break;
        }
        case ExprType::CALL: {
            auto callExpr = dynamic_cast<CallExpr*>(expr.get());
            if (callExpr) {
                // Check the callee
                checkForUndefinedVariables(callExpr->callee);
                // Check all arguments
                for (const auto& arg : callExpr->arguments) {
                    checkForUndefinedVariables(arg);
                }
            }
            break;
        }
        case ExprType::INDEX_ACCESS: {
            auto indexAccessExpr = dynamic_cast<IndexAccessExpr*>(expr.get());
            if (indexAccessExpr) {
                // Check the object being accessed
                checkForUndefinedVariables(indexAccessExpr->object);
                // Check the index expression
                checkForUndefinedVariables(indexAccessExpr->index);
            }
            break;
        }
        default:
            // For literal expressions, no need to check
            break;
    }
}

} // namespace rglite

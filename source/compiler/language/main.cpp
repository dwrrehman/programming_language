
//
//  main.cpp
//  language
//
//  Created by Daniel Rehman on 1901104.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//


/*

 add user cli hooks to make the compiler stop at any stage, and output the internal represetnation to the user.


 like:


 --emit=preprocessed  or    -p
 --emit=ast           or    -ast
 --emit=actiontree   or     -at

 --emit-llvm       or       -llvm



 we need these, they are useful for debugging and they make the compiler alittle bit more featureful.



*/

#include "arguments.hpp"
#include "compiler.hpp"
#include "interpreter.hpp"
#include "error.hpp"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include <vector>
#include <iostream>



//TODO: make the struct file containa  list of flags, which are
// values from an enum class, and the parser, corrector, etc,
// looks at these flags to determine the correct behavior/debug info to give.
// also, this must be completely done in the get cli args function,
// and not change any external interfaces, only extent interfaces.





int main(const int argc, const char** argv) {

    const struct arguments& arguments = get_commandline_arguments(argc, argv);
    if (arguments.error) exit(1);
    else if (arguments.use_interpreter) interpreter(arguments.files[0]);
    else if (!arguments.files.size()) print_error_no_files();

    debug_arguments(arguments);

    llvm::LLVMContext context;
    std::vector<std::unique_ptr<llvm::Module>> modules = {};
    modules.reserve(arguments.files.size());
    bool error = false;

    for (auto file : arguments.files)
        try {modules.push_back(frontend(file, context));}
        catch (...) {error = true;}

    if (error) exit(1);
    for (auto& module : modules) optimize(*module);

    auto& program = pop(modules);
    for (auto& module : modules) link(program, module);
}


/*

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/ModuleSummaryIndex.h"

#include "llvm/AsmParser/Parser.h"
#include "llvm/Support/MemoryBuffer.h"

#include <iostream>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

//===----------------------------------------------------------------------===//
// Lexer
//===----------------------------------------------------------------------===//

// The lexer returns tokens [0-255] if it is an unknown character, otherwise one
// of these for known things.
enum Token {
    tok_eof = -1,

    // commands
    tok_def = -2,
    tok_extern = -3,

    // primary
    tok_identifier = -4,
    tok_number = -5
};

static std::string IdentifierStr; // Filled in if tok_identifier
static double NumVal;             // Filled in if tok_number

/// gettok - Return the next token from standard input.
static int gettok() {

    static int LastChar = ' ';

    // Skip any whitespace.
    while (isspace(LastChar))
        LastChar = getchar();

    if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
        IdentifierStr = LastChar;
        while (isalnum((LastChar = getchar())))
            IdentifierStr += LastChar;

        if (IdentifierStr == "def")
            return tok_def;
        if (IdentifierStr == "extern")
            return tok_extern;
        return tok_identifier;
    }

    if (LastChar == '%') {
        LastChar = getchar();
        return '%';
    }


    if (isdigit(LastChar) || LastChar == '.') { // Number: [0-9.]+
        std::string NumStr;
        do {
            NumStr += LastChar;
            LastChar = getchar();
        } while (isdigit(LastChar) || LastChar == '.');

        NumVal = strtod(NumStr.c_str(), nullptr);
        return tok_number;
    }

    if (LastChar == '#') {
        // Comment until end of line.
        do
            LastChar = getchar();
        while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

        if (LastChar != EOF)
            return gettok();
    }

    // Check for end of file.  Don't eat the EOF.
    if (LastChar == EOF)
        return tok_eof;

    // Otherwise, just return the character as its ascii value.
    int ThisChar = LastChar;
    LastChar = getchar();
    return ThisChar;
}

//===----------------------------------------------------------------------===//
// Abstract Syntax Tree (aka Parse Tree)
//===----------------------------------------------------------------------===//

namespace {

    /// ExprAST - Base class for all expression nodes.
    class ExprAST {
    public:
        virtual ~ExprAST() = default;

        virtual llvm::Value *codegen() = 0;
    };

    /// NumberExprAST - Expression class for numeric literals like "1.0".
    class NumberExprAST : public ExprAST {
        double Val;

    public:
        NumberExprAST(double Val) : Val(Val) {}

        llvm::Value *codegen() override;
    };

    /// VariableExprAST - Expression class for referencing a variable, like "a".
    class VariableExprAST : public ExprAST {
        std::string Name;

    public:
        VariableExprAST(const std::string &Name) : Name(Name) {}

        llvm::Value *codegen() override;
    };

    /// BinaryExprAST - Expression class for a binary operator.
    class BinaryExprAST : public ExprAST {
        char Op;
        std::unique_ptr<ExprAST> LHS, RHS;

    public:
        BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,
                      std::unique_ptr<ExprAST> RHS)
        : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

        llvm::Value *codegen() override;
    };

    /// CallExprAST - Expression class for function calls.
    class CallExprAST : public ExprAST {
        std::string Callee;
        std::vector<std::unique_ptr<ExprAST>> Args;

    public:
        CallExprAST(const std::string &Callee,
                    std::vector<std::unique_ptr<ExprAST>> Args)
        : Callee(Callee), Args(std::move(Args)) {}

        llvm::Value *codegen() override;
    };

    /// PrototypeAST - This class represents the "prototype" for a function,
    /// which captures its name, and its argument names (thus implicitly the number
    /// of arguments the function takes).
    class PrototypeAST {
        std::string Name;
        std::vector<std::string> arguments;

    public:
        PrototypeAST(const std::string &Name, std::vector<std::string> Args)
        : Name(Name), arguments(std::move(Args)) {}

        llvm::Function *codegen();
        const std::string &getName() const { return Name; }
    };

    /// FunctionAST - This class represents a function definition itself.
    class FunctionAST {
        std::unique_ptr<PrototypeAST> Proto;
        std::unique_ptr<ExprAST> Body;

    public:
        FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<ExprAST> Body)
        : Proto(std::move(Proto)), Body(std::move(Body)) {}

        llvm::Function *codegen();
    };

} // end anonymous namespace

//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//

/// CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
/// token the parser is looking at.  getNextToken reads another token from the
/// lexer and updates CurTok with its results.
static int CurTok;
static int getNextToken() { return CurTok = gettok(); }

/// BinopPrecedence - This holds the precedence for each binary operator that is
/// defined.
static std::map<char, int> BinopPrecedence;

/// GetTokPrecedence - Get the precedence of the pending binary operator token.
static int GetTokPrecedence() {
    if (!isascii(CurTok))
        return -1;

    // Make sure it's a declared binop.
    int TokPrec = BinopPrecedence[CurTok];
    if (TokPrec <= 0) return -1;
    return TokPrec;
}

/// LogError* - These are little helper functions for error handling.
std::unique_ptr<ExprAST> LogError(const char *Str) {
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
    LogError(Str);
    return nullptr;
}

static std::unique_ptr<ExprAST> ParseExpression();

/// numberexpr ::= number
static std::unique_ptr<ExprAST> ParseNumberExpr() {
    auto Result = llvm::make_unique<NumberExprAST>(NumVal);
    getNextToken(); // consume the number
    return std::move(Result);
}

/// parenexpr ::= '(' expression ')'
static std::unique_ptr<ExprAST> ParseParenExpr() {
    getNextToken(); // eat (.
    auto expression = ParseExpression();
    if (!expression) return nullptr;

    if (CurTok != ')') return LogError("expected ')'");

    getNextToken(); // eat ).
    return expression;
}

/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'
static std::unique_ptr<ExprAST> ParseIdentifierExpr() {
    std::string IdName = IdentifierStr;

    getNextToken(); // eat identifier.

    if (CurTok != '(') // Simple variable ref.
        return llvm::make_unique<VariableExprAST>(IdName);

    // Call.
    getNextToken(); // eat (
    std::vector<std::unique_ptr<ExprAST>> Args;
    if (CurTok != ')') {
        while (true) {
            if (auto Arg = ParseExpression())
                Args.push_back(std::move(Arg));
            else return nullptr;

            if (CurTok == ')') break;

            if (CurTok != ',') return LogError("Expected ')' or ',' in argument list");
            getNextToken();
        }
    }

    // Eat the ')'.
    getNextToken();

    return llvm::make_unique<CallExprAST>(IdName, std::move(Args));
}

/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
static std::unique_ptr<ExprAST> ParsePrimary() {
    switch (CurTok) {
        default:
            return LogError("unknown token when expecting an expression");
        case tok_identifier:
            return ParseIdentifierExpr();
        case tok_number:
            return ParseNumberExpr();
        case '(':
            return ParseParenExpr();
    }
}

/// binoprhs
///   ::= ('+' primary)*
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
                                              std::unique_ptr<ExprAST> LHS) {
    // If this is a binop, find its precedence.
    while (true) {
        int TokPrec = GetTokPrecedence();

        // If this is a binop that binds at least as tightly as the current binop,
        // consume it, otherwise we are done.
        if (TokPrec < ExprPrec)
            return LHS;

        // Okay, we know this is a binop.
        int BinOp = CurTok;
        getNextToken(); // eat binop

        // Parse the primary expression after the binary operator.
        auto RHS = ParsePrimary();
        if (!RHS)
            return nullptr;

        // If BinOp binds less tightly with RHS than the operator after RHS, let
        // the pending operator take RHS as its LHS.
        int NextPrec = GetTokPrecedence();
        if (TokPrec < NextPrec) {
            RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
            if (!RHS)
                return nullptr;
        }

        // Merge LHS/RHS.
        LHS =
        llvm::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
    }
}

/// expression
///   ::= primary binoprhs
///
static std::unique_ptr<ExprAST> ParseExpression() {
    auto LHS = ParsePrimary();
    if (!LHS)
        return nullptr;

    return ParseBinOpRHS(0, std::move(LHS));
}

/// prototype
///   ::= id '(' id* ')'
static std::unique_ptr<PrototypeAST> ParsePrototype() {
    if (CurTok != tok_identifier)
        return LogErrorP("Expected function name in prototype");

    std::string FnName = IdentifierStr;
    getNextToken();

    if (CurTok != '(')
        return LogErrorP("Expected '(' in prototype");

    std::vector<std::string> ArgNames;
    while (getNextToken() == tok_identifier)
        ArgNames.push_back(IdentifierStr);
    if (CurTok != ')')
        return LogErrorP("Expected ')' in prototype");

    // success.
    getNextToken(); // eat ')'.

    return llvm::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}

/// definition ::= 'def' prototype expression
static std::unique_ptr<FunctionAST> ParseDefinition() {
    getNextToken(); // eat def.

    auto prototype = ParsePrototype();
    if (!prototype) return nullptr;

    if (auto expression = ParseExpression())
        return llvm::make_unique<FunctionAST>(std::move(prototype), std::move(expression));

    return nullptr;
}

/// toplevelexpr ::= expression
static std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
    if (auto expression = ParseExpression()) {
        // Make an anonymous proto.
        auto prototype = llvm::make_unique<PrototypeAST>("__anon_expr", std::vector<std::string>());
        return llvm::make_unique<FunctionAST>(std::move(prototype), std::move(expression));
    }
    return nullptr;
}

/// external ::= 'extern' prototype
static std::unique_ptr<PrototypeAST> ParseExtern() {
    getNextToken(); // eat extern.
    return ParsePrototype();
}

//===----------------------------------------------------------------------===//
// Code Generation
//===----------------------------------------------------------------------===//

static llvm::LLVMContext context;
static llvm::IRBuilder<> Builder(context);
static std::unique_ptr<llvm::Module> TheModule;
static std::map<std::string, llvm::Value*> NamedValues;

llvm::Value *LogErrorV(const char *Str) {
    LogError(Str);
    return nullptr;
}

llvm::Value* NumberExprAST::codegen() {
    return llvm::ConstantFP::get(context, llvm::APFloat(Val));
}

llvm::Value* VariableExprAST::codegen() {
    // Look this variable up in the function.
    llvm::Value* V = NamedValues[Name];
    if (!V) {
        std::cout << Name << ": ";
        return LogErrorV("Unknown variable name");
    }
    return V;
}

llvm::Value *BinaryExprAST::codegen() {
    llvm::Value *L = LHS->codegen();
    llvm::Value *R = RHS->codegen();
    if (!L || !R)
        return nullptr;

    switch (Op) {
        case '+':
            return Builder.CreateFAdd(L, R, "addtmp");
        case '-':
            return Builder.CreateFSub(L, R, "subtmp");
        case '*':
            return Builder.CreateFMul(L, R, "multmp");
        case '<':
            L = Builder.CreateFCmpULT(L, R, "cmptmp");
            // Convert bool 0/1 to double 0.0 or 1.0
            return Builder.CreateUIToFP(L, llvm::Type::getDoubleTy(context), "booltmp");
        default:
            return LogErrorV("invalid binary operator");
    }
}

llvm::Value *CallExprAST::codegen() {
    // Look up the name in the global module table.
    llvm::Function *CalleeF = TheModule->getFunction(Callee);
    if (!CalleeF) {
        std::cout << Callee << ": ";
        return LogErrorV("Unknown function referenced");
    }
    // If argument mismatch error.
    if (CalleeF->arg_size() != Args.size()) {
        std::cout << "Expected " << Args.size() << " arguments, but found " << CalleeF->arg_size() << ".\n";
        return LogErrorV("Incorrect # arguments passed");
    }
    std::vector<llvm::Value *> ArgsV;
    for (unsigned i = 0, e = (unsigned) Args.size(); i != e; ++i) {
        ArgsV.push_back(Args[i]->codegen());
        if (!ArgsV.back()) return nullptr;
    }

    return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
}

llvm::Function *PrototypeAST::codegen() {
    // Make the function type:  double(double,double) etc.
    std::vector<llvm::Type*> parameters(arguments.size(), llvm::Type::getDoubleTy(context));
    llvm::FunctionType* type = llvm::FunctionType::get(llvm::Type::getDoubleTy(context), parameters, false);
    llvm::Function *function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, Name, TheModule.get());

    size_t i = 0;
    for (auto &argument : function->args()) argument.setName(arguments[i++]);

    return function;
}

llvm::Function *FunctionAST::codegen() {
    // First, check for an existing function from a previous 'extern' declaration.
    llvm::Function* function = TheModule->getFunction(Proto->getName());
    if (!function && !(function = Proto->codegen())) return nullptr;

    // Create a new basic block to start insertion into.
    llvm::BasicBlock* block = llvm::BasicBlock::Create(context, "entry", function);
    Builder.SetInsertPoint(block);

    // Record the function arguments in the NamedValues map.
    NamedValues.clear();
    for (auto &Arg : function->args()) NamedValues[Arg.getName()] = &Arg;

    if (llvm::Value* return_value = Body->codegen()) {
        // Finish off the function.
        Builder.CreateRet(return_value);

        // Validate the generated code, checking for consistency.
        verifyFunction(*function);
        return function;
    }

    // Error reading body, remove function.
    function->eraseFromParent();
    return nullptr;
}

//===----------------------------------------------------------------------===//
// Top-Level parsing and JIT Driver
//===----------------------------------------------------------------------===//

static void HandleDefinition() {
    if (auto FnAST = ParseDefinition()) {
        if (auto *FnIR = FnAST->codegen()) {
            fprintf(stderr, "Read function definition:");
            FnIR->print(llvm::errs());
            fprintf(stderr, "\n");
        }
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void HandleExtern() {
    if (auto ProtoAST = ParseExtern()) {
        if (auto *FnIR = ProtoAST->codegen()) {
            fprintf(stderr, "Read extern: ");
            FnIR->print(llvm::errs());
            fprintf(stderr, "\n");
        }
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void HandleTopLevelExpression() {
    // Evaluate a top-level expression into an anonymous function.
    if (auto FnAST = ParseTopLevelExpr()) {
        if (auto *FnIR = FnAST->codegen()) {
            fprintf(stderr, "Read top-level expression:");
            FnIR->print(llvm::errs());
            fprintf(stderr, "\n");
        }
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}



static void otherLoop() {
    std::string s = "";

    while (true) {
        std::cout << "::> ";
        std::getline(std::cin, s);
        std::cout << "received: \"" << s << "\"\n";

        if (s == "done" || s == "") {
            std::cout << "quitting...\n";
            break;
        }

        if (s.size() && s[0]) {
            s.insert(0, "define void @m() {\n"); // wrap the given llvm statements in a function, named the same as the function we want to insert these statements into.
            s += "\nret void\n}\n";
        }

        for (int i = 0; i < s.size(); i++) {    // temp, to make the cli more useable.
            if (s[i] == '/') {
                s[i] = '\n';
            }
        }

        llvm::MemoryBufferRef reference(s, "temp_ins_buffer");
        llvm::ModuleSummaryIndex my_index(true);
        llvm::SMDiagnostic errors;

        if (llvm::parseAssemblyInto(reference, TheModule.get(), &my_index, errors)) {
            std::cout << "\nparsed unsuccessfully.\n\n";

            {
                std::cout << "llvm: "; // TODO: make this have color!
                errors.print("MyProgram.n", llvm::errs());
            }

        } else {
            std::cout << "\nparsed successfully.\n\n";
        }

        std::cout << "printing all functions:\n";
        for (auto& function : TheModule->functions()) {
            std::cout << "printing function:\n";
            function.llvm::Value::print(llvm::errs());
            std::cout << "done printing function!\n";
        }

        std::cout << "now trying to parse s as type!\n";

        llvm::Type* type;

        if ((type = llvm::parseType(s, errors, *TheModule)) != nullptr) {
            std::cout << "succcesfully parsed type: \n";
            type->print(llvm::errs());
            std::cout << "\ndone printing type.\n";
        } else {
            std::cout << "{invalid type.}\n";
        }
        s = "";
    }

    std::cout << "printing the results: \n\n";
    TheModule->print(llvm::errs(), nullptr);
}

/// top ::= definition | external | expression | ';'
static void MainLoop() {
    while (true) {
        fprintf(stderr, "ready> ");
        switch (CurTok) {

            case tok_eof: return;

            case '%':
                otherLoop();
                CurTok = ';';
                break;

            case ';':
                getNextToken();
                break;
            case tok_def:
                HandleDefinition();
                break;
            case tok_extern:
                HandleExtern();
                break;
            default:
                HandleTopLevelExpression();
                break;
        }
    }
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//

int main() {

    // Install standard binary operators.
    // 1 is lowest precedence.
    BinopPrecedence['<'] = 10;
    BinopPrecedence['+'] = 20;
    BinopPrecedence['-'] = 20;
    BinopPrecedence['*'] = 40; // highest.

    // Prime the first token.
    fprintf(stderr, "ready> ");
    getNextToken();

    // Make the module, which holds all the code.
    TheModule = llvm::make_unique<llvm::Module>("My First Module", context);

    // Run the main "interpreter loop" now.
    MainLoop();

    std::cout << "printing the final results: \n\n";
    TheModule->print(llvm::errs(), nullptr);

    return 0;
}

*/

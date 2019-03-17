//
//  nodes.hpp
//  language
//
//  Created by Daniel Rehman on 1902284.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef nodes_hpp
#define nodes_hpp

#include "lexer.hpp"

#include <string>
#include <vector>

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


// base class for ast nodes:
class node {
public:
    bool error = false;
};





// we shouldnt need this....

class required_newlines: public node {
    std::vector<struct token> terminals = {};
};




class interface_declaration: public node {
    node self = {};
};

class implementation_declaration: public node {
    node self = {};
};


class declaration: public node {
    bool interface = false;
    bool implementation = false;
    bool has_documentation = false;
    struct token documentation = {};
    node self = {};
};

class declaration_list: public node {
    std::vector<declaration> declarations = {};
};

class terminated_declaration: public node {
    declaration self = {};
};

class declaration_block: public node {
    std::vector<declaration> declarations = {};
};

class translation_unit: public node {
    bool is_entry_point = false;
    declaration_list list = {};
    //statement_list list = {};
};







/*

/// ExprAST - Base class for all expression nodes.

 class ExprAST {
 public:
    virtual ~ExprAST() = default;
 
    virtual llvm::Value* codegen() = 0;
 };




/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ast_node {
    double Val;
    
public:
    NumberExprAST(double Val) : Val(Val) {}
    
    llvm::Value* codegen() override;
};






/// VariableExprAST - Expression class for referencing a variable, like "a".
class variable_node : public ast_node {
    std::string Name;
    
public:
    variable_node(const std::string &Name) : Name(Name) {}
    
    llvm::Value* codegen() override;
};













/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class signature_node {
    struct signature signature;
    
public:
    signature_node(const std::string &Name, std::vector<std::string> Args)
    : Name(Name), Args(std::move(Args)) {}
    
    llvm::Function* codegen();
    const std::string &getName() const { return Name; }
};



/// FunctionAST - This class represents a function definition itself.
class FunctionAST {
    std::unique_ptr<signature_node> signature;
    std::unique_ptr<ast_node> body;
    
public:
    FunctionAST(std::unique_ptr<signature_node> signature,
                std::unique_ptr<ast_node> Body)
    : signature(std::move(signature)), body(std::move(body)) {}
    
    llvm::Function* codegen();
};


*/

#endif /* nodes_hpp */

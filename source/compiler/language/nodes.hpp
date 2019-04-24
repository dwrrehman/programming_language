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
*/

/* -------------------------- EBNF GRAMMAR FOR THE LANGUAGE: ----------------------------


 translation_unit
 = expression_list

 expression_list
 = newlines terminated_expression expression_list
 | E

 terminated_expression
 = expression required_newlines

 function_signature
 = call_signature return_type signature_type

 variable_signature
 = variable_element_list signature_type

 variable_element_list
 = variable_element variable_element_list

 variable_element
 ...just as a symbol, but with out the variable definition being considered a symbol.

 call_signature
 = ( element_list )

 element_list
 = element element_list
 | E

 element
 = symbol
 | :

 return_type
 = expression
 | E

 signature_type
 = : expression
 | E

 expression
 = symbol expression
 | symbol

 newlines_expression
 = symbol newlines expression
 = symbol

 symbol
 = function_signature
 | variable_signature
 | ( newlines newlines_expression )
 | string_literal
 | character_literal
 | documentation
 | llvm_literal
 | block
 | builtin
 | identifier

 block
 = { expression_list }
 | {{ expression_list }}
 | { expression }
 | {{ expression }}

 ///| newlines expression          ; this is problematic. we will do this as a correction transformation.



----------------------------------------------------------------------------*/

// base class for all ast nodes:

class node {
public:
    bool error = true;
};




// enum classes:

enum class symbol_type {
    none,
    subexpression,
    string_literal,
    character_literal,
    documentation,
    llvm_literal,
    block,
    identifier,
    newline,
    indent,
    abstraction_definition,
    variable_definition,
};




// prototypes:

class translation_unit;

class terminated_expression;
class block;

class expression_list;
class expression;
class symbol;

class variable_signature;
class variable_symbol_list;
class variable_symbol;

class abstraction_signature;
class abstraction_symbol_list;
class abstraction_symbol;



// literals:

class string_literal: public node {
public:
    struct token literal = {};
};

class character_literal: public node {
public:
    struct token literal = {};
};

class documentation: public node {
public:
    struct token literal = {};
};

class llvm_literal: public node {
public:
    struct token literal = {};
};

class identifier: public node {
public:
    struct token name = {};
};

class expression: public node {
public:
    std::vector<symbol> symbols = {};
    size_t indent_level = 0;
};

class expression_list: public node {
public:
    std::vector<expression> expressions = {};
};

class translation_unit: public node {
public:
    expression_list list = {};
};

class block: public node {
public:
    expression_list list = {};
};


/// --------- classes that are used in the corrector: ---------------

class abstraction_definition: public node {
public:
    expression call = {};
    expression return_type = {};
    expression signature_type = {};
    block body = {};
};

class variable_definition: public node {
public:
    expression name = {};
    expression type = {};
};
/// ----------------------------------------------------------------


class symbol: public node {
public:
    enum symbol_type type = symbol_type::none;
    expression subexpression = {};
    block block = {};
    string_literal string = {};
    character_literal character = {};
    documentation documentation = {};
    llvm_literal llvm = {};
    identifier identifier = {};
    abstraction_definition abstraction = {};
    variable_definition variable = {};

    symbol(){}
    symbol(enum symbol_type type) {this->type = type;}
};




#endif /* nodes_hpp */

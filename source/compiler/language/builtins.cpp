//
//  builtins.cpp
//  language
//
//  Created by Daniel Rehman on 1905175.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "builtins.hpp"
#include <vector>
#include "nodes.hpp"

expression failure = {true};

/// Global builtin types. these are fundemental to the language:
expression type_type = {{{"_type", false}}};
expression unit_type = {{}, &type_type};
expression none_type = {{{"_none", false}}, &type_type};
expression infered_type = {{{"_infered", false}}};

expression expression_type = {{{"_expression", false}, {{{}, &type_type}}}, &type_type};        // an unevaluated expression,   with a type.
expression signature_type = {{{"_signature", false}}, &type_type};                              // an undefined expression,     with no type.

expression application_type = {{{"_application", false}}, &type_type};
expression abstraction_type = {{{"_abstraction", false}}, &type_type};

expression compiletime_type = { { {"_compiletime", false}, {{{}, &type_type}} }, &type_type};
expression runtime_type = { { {"_runtime", false}, {{{}, &type_type}} }, &type_type};

expression immutable_type = { { {"_immutable", false}, {{{}, &type_type}} }, &type_type};
expression mutable_type = { { {"_mutable", false}, {{{}, &type_type}} }, &type_type};

/// global builtin abstractions:

expression define_abstraction = {
    {
        {"_define", false}, {{{}, &signature_type}},
        {"as", false}, {{{}, &expression_type}},
        {"in", false}, {{{}, &application_type}},
    }, &unit_type};

expression undefine_abstraction = {
    {
        {"_undefine", false}, {{{}, &expression_type}},
        {"in", false}, {{{}, &application_type}},
    }, &unit_type};

expression disclose_abstraction = {
    {
        {"_disclose", false}, {{{}, &signature_type}},
        {"from", false}, {{{}, &expression_type}},
        {"into", false}, {{{}, &application_type}},
    }, &unit_type};

/// global builtin signatures:

expression all_signature = {
    {
        {"_all", false}
    }, &signature_type};



std::vector<expression> builtins =  {
    type_type, unit_type, none_type, infered_type,

    signature_type,     expression_type, 
    abstraction_type,   application_type,    
    compiletime_type,   runtime_type,
    immutable_type,     mutable_type, 

    define_abstraction, undefine_abstraction,
    disclose_abstraction, all_signature,                                           
};






///TODO: still missing:
/**
 
 _precedence <_signature>
 _associativity <_signature>
 
 _recognize <_type> named <_signature>
 _recognize <_signature>
 
 _resolve <_expression> at compiletime
 _resolve <_expression> at runtime 
 
 _find <_signature> in <_application>
 
 _declare external <_signature> from <_language>         ; revise this.
 
 */

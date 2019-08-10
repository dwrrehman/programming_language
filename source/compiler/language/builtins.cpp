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

expression type_type = {{{"_type", false}}}; // has no type.
expression infered_type = {{{"_infered", false}}}; // has no type.

expression unit_type = {{}, &type_type};
expression none_type = {{{"_none", false}}, &type_type};
expression any_type = {{{"_any", false}}, &type_type};

expression signature_type = {{{"_signature", false}}, &type_type};     
expression application_type = {{{"_application", false}}, &type_type};
expression abstraction_type = {{{"_abstraction", false}}, &type_type};

expression unevaluated_type = {{{"_unevaluated", false}, {{{}, &type_type}}}, &type_type};     
expression compiletime_type = { { {"_compiletime", false}, {{{}, &type_type}} }, &type_type};
expression runtime_type = { { {"_runtime", false}, {{{}, &type_type}} }, &type_type};

expression immutable_type = { { {"_immutable", false}, {{{}, &type_type}} }, &type_type};
expression mutable_type = { { {"_mutable", false}, {{{}, &type_type}} }, &type_type};

/// global builtin abstractions:

expression define_abstraction = {
    {
        {"_define", false}, {{{}, &signature_type}},
        {"as", false}, {{{}, &unevaluated_type}},
        {"in", false}, {{{}, &application_type}},
    }, &unit_type};

expression undefine_abstraction = {
    {
        {"_undefine", false}, {{{}, &unevaluated_type}},
        {"in", false}, {{{}, &application_type}},
    }, &unit_type};

expression disclose_abstraction = {
    {
        {"_disclose", false}, {{{}, &signature_type}},
        {"from", false}, {{{}, &unevaluated_type}},
        {"into", false}, {{{}, &application_type}},
    }, &unit_type};

expression all_signature = {{{"_all", false}}, &type_type};


std::vector<expression> builtins =  {
    type_type, unit_type, none_type, any_type, infered_type,
    
    signature_type,     unevaluated_type,
    abstraction_type,   application_type,
    compiletime_type,   runtime_type,
    immutable_type,     mutable_type,
    
    define_abstraction, undefine_abstraction,
    disclose_abstraction,
    all_signature,
};






///TODO: still missing:
/**
 
 
 
 
 
 // important:
 
        _resolve <_expression> at compiletime
        _resolve <_expression> at runtime 
 
 
 // not as important...
 
        _precedence <_signature>
        _associativity <_signature>
 
        _find <_signature> in <_application>
 
 
 
 
 */

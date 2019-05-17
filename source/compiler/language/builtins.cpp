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

/// Global builtin types. these are fundemental to the language:
expression type_type = {{{"_type", false}}};
expression unit_type = {{}, &type_type};
expression none_type = {{{"_none", false}}, &type_type};
expression infered_type = {{{"_infered", false}}};

expression expression_type = {{{"_expression", false}}, &type_type};
expression signature_type = {{{"_signature", false}}, &type_type};
expression block_type = {{{"_block", false}}, &type_type};

expression application_type = {{{"_application", false}}, &type_type};
expression abstraction_type = {{{"_abstraction", false}}, &type_type};

expression compiletime_type = { { {"_compiletime", false}, {{{}, &type_type}} }, &type_type};
expression runtime_type = { { {"_runtime", false}, {{{}, &type_type}} }, &type_type};
expression compiletime_abstraction = { { {"_compiletime", false}, {{{}, &expression_type}} }, &unit_type};
expression runtime_abstraction = { { {"_runtime", false}, {{{}, &expression_type}} }, &unit_type};

expression immutable_type = { { {"_immutable", false}, {{{}, &type_type}} }, &type_type};
expression mutable_type = { { {"_mutable", false}, {{{}, &type_type}} }, &type_type};


expression define_abstraction = {
    {
        {"_define", false},
        {{{}, &signature_type}},
        {"as", false},
        {{{}, &expression_type}},
        {"in", false},
        {{{}, &application_type}},
    }, &unit_type};

expression undefine_abstraction = {
    {
        {"_undefine", false},
        {{{}, &expression_type}},
        {"in", false},
        {{{}, &application_type}},
    }, &unit_type};


expression undefine_all_abstraction = {
    {
        {"_undefine", false},
        {"all", false},
        {"in", false},
        {{{}, &application_type}},
    }, &unit_type};


expression disclose_abstraction = {
    {
        {"_disclose", false},
        {{{}, &expression_type}},
        {"from", false},
        {{{}, &expression_type}},
        {"into", false},
        {{{}, &application_type}},
    }, &unit_type};

expression disclose_all_abstraction = {
    {
        {"_disclose", false},
        {"all", false},
        {"from", false},
        {{{}, &expression_type}},
        {"into", false},
        {{{}, &application_type}},
    }, &unit_type};


///TODO: still missing _precedence, _associativity, and possibly one more...

std::vector<expression> builtins =  {
    type_type, unit_type, none_type, infered_type,

    expression_type, block_type, signature_type,
    application_type, abstraction_type,

    compiletime_type, runtime_type,
    compiletime_abstraction, runtime_abstraction,

    define_abstraction, undefine_abstraction, undefine_all_abstraction,
    disclose_abstraction, disclose_all_abstraction,
};

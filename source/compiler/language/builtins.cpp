//
//  builtins.cpp
//  language
//
//  Created by Daniel Rehman on 1905175.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "builtins.hpp"

expression failure = {true};

/// Global builtin types. these are fundemental to the language:

/**
 
 
    0  :  {typeless}
    
    1  :  type type
 
    2  :  infered type
 
    3  :  unit type
 
    4  :  none type
    
    5  :  any type
 
    6  :  application type
 
    7  :  abstraction type
 
    8  :  compiletime type
    
    9  :  runtime type
 
    10  :  immutable type
 
    11  :  mutable type 
 
    12  :  define abstraction
 
    13  :  undefine abstraction
  
    14  :  disclose abstraction
       
    15  :  all signature
 
 */

expression type_type = {{{"_type", false}}}; // has no type.
expression infered_type = {{{"_infered", false}}}; // has no type.
expression unit_type = {{}, 1};
expression none_type = {{{"_none", false}}, 1};
expression any_type = {{{"_any", false}}, 1};
     
expression application_type = {{{"_application", false}}, 1};
expression abstraction_type = {{{"_abstraction", false}}, 1};
     
expression compiletime_type = { { {"_compiletime", false}, {{{},1}}},1};
expression runtime_type = { { {"_runtime", false}, {{{},1}}},1};

expression immutable_type = { { {"_immutable", false}, {{{},1}}},1};
expression mutable_type = { { {"_mutable", false}, {{{},1}}},1};

/// global builtin abstractions:

expression define_abstraction = {
    {
        {"_define", false}, {{{}, 7}},
        {"as", false}, {{{}, 6}},
        {"in", false}, {{{}, 6}},
    }, 3};

expression undefine_abstraction = {
    {
        {"_undefine", false}, {{{}, 6}},
        {"in", false}, {{{}, 6}},
    }, 3};

expression disclose_abstraction = {
    {
        {"_disclose", false}, {{{}, 7}},
        {"from", false}, {{{}, 6}},
        {"into", false}, {{{}, 6}},
    }, 3};

expression all_signature = {{{"_all", false}}, 7};


std::vector<expression> builtins =  {
    type_type, infered_type, unit_type, none_type, any_type, 
    
    application_type, abstraction_type,
    compiletime_type,   runtime_type,
    immutable_type,     mutable_type,
    
    define_abstraction, undefine_abstraction, disclose_abstraction,
    all_signature,
};






///TODO: still missing:
/**
 
 
 
 
 
 to add:
 
    
 
        _instantiate
 
 
 
 
 
 
 
 
 // important:
 
        _resolve <_expression> at compiletime
        _resolve <_expression> at runtime 
 
 
 // not as important...
 
        _precedence <_signature>
        _associativity <_signature>
 
        _find <_signature> in <_application>
 
 
 
 
 */

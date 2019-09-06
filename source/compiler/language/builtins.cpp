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
 
 0  :  {typeless}               : 0
 1  :  infered type             : 0
 2  :  type type                : 0
 3  :  none type                : type
 4  :  unit type                : type
 
 5  :  application type         : type
 6  :  abstraction type         : type  
 7  :  create                   : unit
 8  :  define                   : unit
 9  :  expose                   : unit
 
 10  : ct                       : type  
 11  : rt                       : type
 12  : imm                      : type
 13  : mut                      : type
 
 */
expression infered_type = {{{"__", false}}}; // has no type.
expression type_type = {{{"_type", false}}}; // has no type.
expression unit_type = {{{"_unit", false}}, intrin::type};
expression none_type = {{{"_none", false}}, intrin::type};
expression application_type = {{{"_a", false}}, intrin::type};
expression abstraction_type = {{{"_b", false}}, intrin::type};

expression create_abstraction = {
    {
        {"_c", false},  {{{}, intrin::application}},        
    }, intrin::unit};

expression define_abstraction = {
    {
        {"_d", false}, {{{}, intrin::abstraction}},
        {{{}, intrin::application}},
        {{{}, intrin::application}},
        {{{}, intrin::application}},
    }, intrin::unit};

expression expose_abstraction = {
    {
        {"_e", false}, {{{}, intrin::application}},
        {{{}, intrin::abstraction}},
        {{{}, intrin::application}},
    }, intrin::unit};






/// FIX ME
expression compiletime_type = { { {"_compiletime", false}, {{{},intrin::type}}},intrin::type};
expression runtime_type = { { {"_runtime", false}, {{{},intrin::type}}},intrin::type};
/// FIX ME
expression immutable_type = { { {"_immutable", false}, {{{},intrin::type}}},intrin::type};
expression mutable_type = { { {"_mutable", false}, {{{},intrin::type}}},intrin::type};





std::vector<expression> builtins =  {
    infered_type, type_type, none_type, unit_type,
    application_type, abstraction_type,
    create_abstraction, define_abstraction, expose_abstraction,
        
    compiletime_type,   runtime_type,
    immutable_type,     mutable_type,    
};

std::string stringify_intrin(size_t i) {
    switch (i) {
        case intrin::typeless: return "typeless";
        case intrin::infered: return "infered";
        case intrin::type: return "type";                    
        case intrin::none: return "none";
        case intrin::unit: return "unit";
            
        case intrin::application: return "application";
        case intrin::abstraction: return "abstraction";
        case intrin::define: return "define";                                
        case intrin::expose: return "expose";
        case intrin::create: return "create";
            
        case intrin::compiletime: return "compiletime";
        case intrin::runtime: return "runtime";        
        case intrin::immutable: return "immutable";
        case intrin::mutable_: return "mutable";
    }
    return "{compiler error}";
}






///TODO: still missing:
/**
 
 
 
 
 
 to add:
 
    
 
        _instantiate <type>
 
    
 
 
 
 
 
 
 // important:
 
        _resolve <_expression> at compiletime
        _resolve <_expression> at runtime 
 
 
 // not as important...
 
        _precedence <_signature>
        _associativity <_signature>
 
        _find <_signature> in <_application>
 
 
 
 
 */

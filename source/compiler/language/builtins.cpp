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
expression unit_type = {{}, intrin::type};
expression none_type = {{{"_none", false}}, intrin::type};
expression any_type = {{{"_any", false}}, intrin::type};
     
expression application_type = {{{"_application", false}}, intrin::type};
expression abstraction_type = {{{"_abstraction", false}}, intrin::type};
     
expression compiletime_type = { { {"_compiletime", false}, {{{},intrin::type}}},intrin::type};
expression runtime_type = { { {"_runtime", false}, {{{},intrin::type}}},intrin::type};

expression immutable_type = { { {"_immutable", false}, {{{},intrin::type}}},intrin::type};
expression mutable_type = { { {"_mutable", false}, {{{},intrin::type}}},intrin::type};

/// global builtin abstractions:

expression define_abstraction = {
    {
        {"_define", false}, {{{}, intrin::abstraction}},
        {"as", false}, {{{}, intrin::application}},
        {"in", false}, {{{}, intrin::application}},
    }, intrin::unit};

expression undefine_abstraction = {
    {
        {"_undefine", false}, {{{}, intrin::application}},
        {"in", false}, {{{}, intrin::application}},
    }, intrin::unit};

expression disclose_abstraction = {
    {
        {"_disclose", false}, {{{}, intrin::abstraction}},
        {"from", false}, {{{}, intrin::application}},
        {"into", false}, {{{}, intrin::application}},
    }, intrin::unit};

expression all_signature = {{{"_all", false}}, intrin::abstraction};


std::vector<expression> builtins =  {
    type_type, infered_type, unit_type, none_type, any_type, 
    
    application_type, abstraction_type,
    compiletime_type,   runtime_type,
    immutable_type,     mutable_type,
    
    define_abstraction, undefine_abstraction, disclose_abstraction,
    all_signature,
    
};





std::string stringify_intrin(size_t i) {
    switch (i) {
        case intrin::typeless: return "typeless";
        case intrin::type: return "type";
        case intrin::infered: return "infered";
        case intrin::unit: return "unit";    
        case intrin::none: return "none";
        case intrin::any: return "any";
        case intrin::application: return "application";
        case intrin::abstraction: return "abstraction";
        case intrin::compiletime: return "compiletime";
        case intrin::runtime: return "runtime";        
        case intrin::immutable: return "immutable";
        case intrin::mutable_: return "mutable";
        case intrin::define: return "define";            
        case intrin::undefine: return "undefine";            
        case intrin::disclose: return "disclose";            
        case intrin::all: return "all";            
    }
    return "uh oh.";
}






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

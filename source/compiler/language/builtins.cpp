//
//  builtins.cpp
//  language
//
//  Created by Daniel Rehman on 1905175.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "builtins.hpp"



/// Global builtin types. these are fundemental to the language:

/**
 
 0  :  {typeless}               : 0
 1  :  infered type    __       : 0
 2  :  type type       _type    : 0
 3  :  none type       _0       : type
 4  :  unit type       _1       : type
 
 5  :  application type   _a    : type
 6  :  abstraction type   _b    : type  
 7  :  create             _c    : unit
 8  :  define             _d    : unit
 9  :  expose             _e    : unit
 
 10  : ct                       : type  
 11  : rt                       : type
 12  : imm                      : type
 13  : mut                      : type
 
 */

expression failure = {true, true, true};
expression infered_type = {{symbol{"__"}}};     // has no type.
expression type_type = {{symbol{"_type"}}};     // has no type.
expression none_type = {{symbol{"_0"}}, intrin::type};
expression unit_type = {{symbol{"_1"}}, intrin::type};
expression application_type = {{symbol{"_a"}}, intrin::type};
expression abstraction_type = {{symbol{"_b"}}, intrin::type};

expression create_abstraction = {
    std::vector<symbol> {
        //symbol {"_c"},
        //symbol {{expression{intrin::type}}},
    }, intrin::unit};

expression define_abstraction = {
    std::vector<symbol> {
        //symbol{"_d"},
        //{{expression{intrin::abstraction}}},
        //{{expression{intrin::type}}},
        //{{expression{intrin::application}}},
        //{{expression{intrin::application}}},
    }, intrin::unit};

expression expose_abstraction = {
    std::vector<symbol> {
//        {"_e"},
//        {{expression{intrin::application}}},
//        {{expression{intrin::abstraction}}},
//        {{expression{intrin::application}}},
    }, intrin::unit};


/// FIX ME
//expression compiletime_type = { { {"_compiletime"}, {{{},intrin::type}}},intrin::type};
//expression runtime_type = { { {"_runtime"}, {{{},intrin::type}}},intrin::type};
/// FIX ME
//expression immutable_type = { { {"_immutable"}, {{{},intrin::type}}},intrin::type};
//expression mutable_type = { { {"_mutable"}, {{{},intrin::type}}},intrin::type};



std::vector<expression> builtins =  {
    infered_type, type_type, none_type, unit_type,
    application_type, abstraction_type,
    create_abstraction, define_abstraction, expose_abstraction,
        
//    compiletime_type,   runtime_type,
//    immutable_type,     mutable_type,    
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
            
//        case intrin::compiletime: return "compiletime";
//        case intrin::runtime: return "runtime";        
//        case intrin::immutable: return "immutable";
//        case intrin::mutable_: return "mutable";
    }
    return "{compiler error}";
}


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

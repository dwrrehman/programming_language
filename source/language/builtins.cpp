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
 
 
 
 
 
 0  :    {typeless}               : 0
 
 1  :    infered type    _?       : 0
 
 2  :    type type       _        : 0
 
 3  :    none type       _0       : type
 4  :    unit type       _1       : type
 
 4.5  :  unit value       ()       : unit
 
 5  :    application type   _a    : type                        ---> how we pass around unevalatued code.
 6  :    abstraction type   _b    : type                        ---> how we pass around undefined signatures.
 
 7  :    create             _c    : 0             <---- returns something of type T.
 
 8  :    define             _d    : unit
 9  :    expose             _e    : unit        
 
 
 
 
 
 ten intrinsics:
 
                        _   _?   _0   _1   ()   _a   _b   _c   _d   _e 
 
    
 
 this is all you need, actually:
 
 
                                _     _1    ()    _d    _c     
 
 
        and really, techncailly, you only need: 
    
 
                        _d                 in order to be turing complete...?
 
    
        
      
 
 expression define_abstraction = {
     {
         {{"_d"}}, {{intrin::abstraction}}, // signature
         {{intrin::type}}, // of type
         {{intrin::application}}, // as definition 
         {{intrin::application}}, // into scope
     }, intrin::unit};

 expression expose_abstraction = {
     {
         {{"_e"}}, {{intrin::application}}, // existing signature
         {{intrin::abstraction}}, // as alias
         {{intrin::application}}, // from parent        
         {{intrin::application}}, // into scope
     }, intrin::unit};


 
 
_define <signature _b> of <type _> as <definition _a> into  <scope>

_expose <existing_signature _a> from <parent_abstraction> into <scope>

 
 
 
 
 /// DELETE ME:
 
    10  : ct                       : type  
    11  : rt                       : type
    12  : imm                      : type
    13  : mut                      : type
 
 */

expression failure = {true, true, true};
expression infered_type = {{{{"__"}}}};     // has no type.
expression type_type = {{{{"_"}}}};         // has no type.
expression none_type = {{{{"_0"}}}, intrin::type};
expression unit_type = {{{{"_1"}}}, intrin::type};
expression unit_value = {{}, intrin::unit};

expression application_type = {{{{"_a"}}}, intrin::type};
expression abstraction_type = {{{{"_b"}}}, intrin::type};
expression create_abstraction = {{{{"_c"}}, {{intrin::type}}}, intrin::unit};

expression define_abstraction = {
    {
        {{"_d"}}, {{intrin::abstraction}}, // signature
        {{intrin::type}}, // of type
        {{intrin::application}}, // as definition 
        {{intrin::application}}, // into scope
    }, intrin::unit};

expression expose_abstraction = {
    {
        {{"_e"}}, {{intrin::application}}, // existing signature
        // {{intrin::abstraction}}, // as alias                             // dont over compilcate it....?
        {{intrin::application}}, // from parent        
        {{intrin::application}}, // into scope
    }, intrin::unit};



/////////////////////// test (for csr suite) abstractions //////////////////////////

expression hello_abstraction = {
    {
        {{"use"}}, {{intrin::unit}}, {{"-"}}, {{">"}},{{"hello"}}, 
    }, intrin::unit}; 

expression dog_abstraction = {
    {
        {{"dog"}}, 
    }, intrin::unit};


expression A_abstraction = {
    {
        {{"A"}}, 
    }, intrin::application};

expression B_abstraction = {
    {
        {{"B"}}, 
    }, intrin::abstraction};


expression C_abstraction = {
    {
        {{"C"}}, 
    }, intrin::empty};



expression my_define_abstraction = {
    {
        {{"define"}}, {{intrin::abstraction}}, {{"and"}}, {{"poop"}}, {{intrin::abstraction}}, {{"stop"}}, {{"it"}}
    }, intrin::unit};




/// FIX ME
//expression compiletime_type = { { {"_compiletime"}, {{{},intrin::type}}},intrin::type};
//expression runtime_type = { { {"_runtime"}, {{{},intrin::type}}},intrin::type};
/// FIX ME
//expression immutable_type = { { {"_immutable"}, {{{},intrin::type}}},intrin::type};
//expression mutable_type = { { {"_mutable"}, {{{},intrin::type}}},intrin::type};


std::vector<expression> builtins =  {
    infered_type, type_type, none_type, unit_type, unit_value,
    application_type, abstraction_type, 
    create_abstraction, define_abstraction, expose_abstraction, 
    
    hello_abstraction,
    dog_abstraction, A_abstraction, B_abstraction, C_abstraction,
    my_define_abstraction,        
};

std::string stringify_intrin(size_t i) {
    switch (i) {
        case intrin::typeless: return "typeless";
                    
        case intrin::type: return "_";
        case intrin::infered: return "_?";
        case intrin::none: return "_0";
        case intrin::unit: return "_1";
        case intrin::empty: return "()";
            
        case intrin::application: return "_a";
        case intrin::abstraction: return "_b";
        case intrin::create: return "_c";
        case intrin::define: return "_d";
        case intrin::expose: return "_e";
        /// case intrin::equals: return "_f";             // do we need this?
    }
    return "{compiler error}";
}

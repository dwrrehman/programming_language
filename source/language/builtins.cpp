#include "builtins.hpp"

/** --------------- intrins ----------------

0  :        {typeless}          :
1  :    __  infered type        :
2  :    _   type type           :
3  :    _0  none type           : _
4  :    _1  unit type           : _
5  :    ()  unit value          : _1
5  :    _a  application type    : _
6  :    _b  abstraction type    : _
7  :    _c  define signature    : _1
 
 _a         --->        how we pass around unevalatued code.
 _b         --->        how we pass around undefined signatures.
*/

expression failure = {true, true, true};
expression infered_type = {{{{"__"}}}, intrin::typeless};  
expression type_type = {{{{"_"}}}, intrin::typeless};
expression none_type = {{{{"_0"}}}, intrin::type};
expression unit_type = {{{{"_1"}}}, intrin::type};
expression unit_value = {{}, intrin::unit};
expression llvm_type = {{{{"_llvm"}}}, intrin::typeless};    // identical to type_type, but simply a placeholder for me.
expression application_type = {{{{"_a"}}}, intrin::type};
expression abstraction_type = {{{{"_b"}}}, intrin::type};
expression define_abstraction = {
    {
        {{"_c"}}, {{intrin::abstraction}}, // signature
        {{intrin::type}},           // of type
        {{intrin::application}}, // as definition 
        {{intrin::application}}, // into scope
    }, intrin::unit};

std::vector<expression> builtins =  {
     type_type, infered_type, none_type, unit_type, unit_value, llvm_type,
    application_type, abstraction_type, define_abstraction
};

std::string stringify_intrin(nat i) {
    switch (i) {
        case intrin::typeless: return "typeless";
        case intrin::type: return "_";
        case intrin::infered: return "__";
        case intrin::none: return "_0";
        case intrin::unit: return "_1";
        case intrin::unit_value: return "()";
        case intrin::llvm: return "_llvm";
            
        case intrin::application: return "_a";
        case intrin::abstraction: return "_b";
        case intrin::define: return "_c";
    }
    return "{compiler error}";
}

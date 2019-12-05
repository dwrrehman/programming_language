#ifndef builtins_hpp
#define builtins_hpp

#include "nodes.hpp"

#include <vector>

namespace intrin {
    enum intrin_name_index { // index into master.     /// ORDER MATTERS.
        typeless, type, infered,
        none, unit, unit_value, llvm,
        application, abstraction, define
    };
}

extern expression failure;
extern expression infered_type;
extern expression type_type;
extern expression none_type;
extern expression unit_type;
extern expression unit_value;
extern expression application_type;
extern expression abstraction_type;
extern expression define_abstraction;
extern std::vector<expression> builtins;

std::string stringify_intrin(nat i);

#endif /* builtins_hpp */

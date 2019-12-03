//
//  builtins.hpp
//  language
//
//  Created by Daniel Rehman on 1905175.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef builtins_hpp
#define builtins_hpp

#include "nodes.hpp"

#include <vector>

/**
 
 0  :  {typeless}
 1  :  infered type
 2  :  type type 
 3  :  none type 
 4  :  unit type
 
 5  :  application type 
 6  :  abstraction type 
 
 7  :  define
 */
namespace intrin {
    enum intrin_name_index { // index into master.     /// ORDER MATTERS.
        typeless,
        type,
        infered,
        none,
        unit,        
        empty,
        llvm,
        
        application,
        abstraction,
        
        define,
        print,
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

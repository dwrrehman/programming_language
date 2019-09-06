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
 7  :  create 
 8  :  define    
 9  :  expose
 
 10  : ct  
 11  : rt 
 12  : imm
 13  : mut 
 
 */
namespace intrin {
    enum intrin_name_index {
        typeless,
        infered,
        type,
        none,
        unit,
        
        application,
        abstraction,
        create,
        define,        
        expose,
        
        compiletime,
        runtime,
        immutable,
        mutable_,
    };
}

extern expression failure;
extern expression infered_type;
extern expression type_type;
extern expression unit_type;
extern expression none_type;

extern expression application_type;
extern expression abstraction_type;
extern expression create_abstraction;
extern expression define_abstraction;
extern expression expose_abstraction;

// FIX ME:
extern expression compiletime_type;
extern expression runtime_type;
extern expression immutable_type;
extern expression mutable_type;

extern std::vector<expression> builtins;

std::string stringify_intrin(size_t i);

#endif /* builtins_hpp */

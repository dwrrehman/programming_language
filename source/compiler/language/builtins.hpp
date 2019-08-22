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
namespace intrin {
    enum intrin_name_index {
        typeless,
        type,
        infered,
        unit,
        none,
        any,
        application,
        abstraction,
        compiletime,
        runtime,
        immutable,
        mutable_,
        define,
        undefine,
        disclose,
        all,
    };
}

extern expression failure;

extern expression type_type;
extern expression unit_type;
extern expression none_type;
extern expression any_type;
extern expression infered_type;

extern expression application_type;
extern expression abstraction_type;

extern expression compiletime_type;
extern expression runtime_type;

extern expression immutable_type;
extern expression mutable_type;

extern expression define_abstraction;
extern expression undefine_abstraction;
extern expression disclose_abstraction;

extern expression all_signature;

extern std::vector<expression> builtins;


std::string stringify_intrin(size_t i);

#endif /* builtins_hpp */

//
//  corrector.hpp
//  language
//
//  Created by Daniel Rehman on 1903192.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef corrector_hpp
#define corrector_hpp

#include "parser.hpp"
#include "nodes.hpp"

class assignment_statement: public node {
    // signature s = {}       // ???
    expression value = {};
};

translation_unit correct_ast(translation_unit unit);

#endif /* corrector_hpp */

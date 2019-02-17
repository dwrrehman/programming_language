//
//  expressions.hpp
//  language
//
//  Created by Daniel Rehman on 1902096.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef expressions_hpp
#define expressions_hpp

#include "parser.hpp"

expression_node parse_expression(node input, std::vector<struct function_signature> current_signatures);

#endif /* expressions_hpp */

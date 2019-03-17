//
//  analysis.hpp
//  language
//
//  Created by Daniel Rehman on 1901314.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef analysis_hpp
#define analysis_hpp

#include "parser.hpp"
#include "expressions.hpp"
#include "nodes.hpp"

#include <vector>
#include <string>

struct action_tree {
    int undefined;
};

struct action_tree analyze(program tree, std::string filename);

#endif /* analysis_hpp */

//
//  analysis.hpp
//  language
//
//  Created by Daniel Rehman on 1901314.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef analysis_hpp
#define analysis_hpp

#include <vector>
#include <string>

#include "parser.hpp"
#include "expressions.hpp"

node analyze(node tree, bool &error);

#endif /* analysis_hpp */

//
//  arguments.hpp
//  language
//
//  Created by Daniel Rehman on 1902262.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef arguments_hpp
#define arguments_hpp

#include <iostream>
#include <fstream>
#include <vector>

struct arguments get_commandline_arguments(int argc, const char** argv);
void debug_arguments(struct arguments args);

#endif /* arguments_hpp */

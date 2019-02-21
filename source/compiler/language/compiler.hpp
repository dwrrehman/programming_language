//
//  compiler.hpp
//  language
//
//  Created by Daniel Rehman on 1901104.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef compiler_hpp
#define compiler_hpp

#include <string>

std::string get_file(std::string filepath);
void frontend(std::string filename, std::string text);

#endif /* compiler_hpp */

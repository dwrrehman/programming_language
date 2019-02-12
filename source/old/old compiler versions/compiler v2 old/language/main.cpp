//
//  main.cpp
//  language
//
//  Created by Daniel Rehman on 1901104.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include <iostream>
#include <fstream>

#include "compiler.hpp"

const std::string filepath = "/Users/deniylreimn/Documents/projects/programming language/compiler v2/language/test.lang";

int main(int argc, const char * argv[]) {
    auto text = get_file(filepath);
    frontend(text);
    return 0;
}

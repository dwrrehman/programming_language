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

const std::string filepath = "/Users/deniylreimn/Documents/projects/programming language/source/compiler/language/test.lang";

int main(int argc, const char * argv[]) {
    
    // arguments = get_commandline_arguments(argc, argv);
    auto text = get_file(filepath); // temp
    // do_something_based_on(arguments);
    frontend("filename", text); // tree = frontend();
    // backend(tree);
    // link, or do something now...
    
    return 0;
}
